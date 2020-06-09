// @(#)root/thread:$Id$
// Author: Danilo Piparo, CERN  11/2/2016

/*************************************************************************
 * Copyright (C) 1995-2018, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_TThreadedObject
#define ROOT_TThreadedObject

#include "TList.h"
#include "TError.h"

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "ROOT/TSpinMutex.hxx"
#include "TROOT.h"

class TH1;

namespace ROOT {

   namespace Internal {

      namespace TThreadedObjectUtils {

         /// Get the unique index identifying a TThreadedObject.
         inline unsigned GetTThreadedObjectIndex() {
            static unsigned fgTThreadedObjectIndex = 0;
            return fgTThreadedObjectIndex++;
         }

         template<typename T, bool ISHISTO = std::is_base_of<TH1,T>::value>
         struct Detacher{
            static T* Detach(T* obj) {
               return obj;
            }
         };

         template<typename T>
         struct Detacher<T, true>{
            static T* Detach(T* obj) {
               obj->SetDirectory(nullptr);
               obj->ResetBit(kMustCleanup);
               return obj;
            }
         };

         /// Return a copy of the object or a "Clone" if the copy constructor is not implemented.
         template<class T, bool isCopyConstructible = std::is_copy_constructible<T>::value>
         struct Cloner {
            static T *Clone(const T *obj, TDirectory* d = nullptr) {
               T* clone;
               if (d){
                  TDirectory::TContext ctxt(d);
                  clone = new T(*obj);
               } else {
                  clone = new T(*obj);
               }
               return Detacher<T>::Detach(clone);
            }
         };

         template<class T>
         struct Cloner<T, false> {
            static T *Clone(const T *obj, TDirectory* d = nullptr) {
               T* clone;
               if (d){
                  TDirectory::TContext ctxt(d);
                  clone = (T*)obj->Clone();
               } else {
                  clone = (T*)obj->Clone();
               }
               return clone;
            }
         };

         template<class T, bool ISHISTO = std::is_base_of<TH1,T>::value>
         struct DirCreator{
            static std::vector<TDirectory*> Create(unsigned maxSlots) {
               std::string dirName = "__TThreaded_dir_";
               dirName += std::to_string(ROOT::Internal::TThreadedObjectUtils::GetTThreadedObjectIndex()) + "_";
               std::vector<TDirectory*> dirs;
               dirs.reserve(maxSlots);
               for (unsigned i=0; i< maxSlots;++i) {
                  auto dir = gROOT->mkdir((dirName+std::to_string(i)).c_str());
                  dirs.emplace_back(dir);
               }
               return dirs;
            }
         };

         template<class T>
         struct DirCreator<T, true>{
            static std::vector<TDirectory*> Create(unsigned maxSlots) {
               std::vector<TDirectory*> dirs(maxSlots, nullptr);
               return dirs;
            }
         };

      } // End of namespace TThreadedObjectUtils
   } // End of namespace Internals

   namespace TThreadedObjectUtils {

      template<class T>
      using MergeFunctionType = std::function<void(std::shared_ptr<T>, std::vector<std::shared_ptr<T>>&)>;
      /// Merge TObjects
      template<class T>
      void MergeTObjects(std::shared_ptr<T> target, std::vector<std::shared_ptr<T>> &objs)
      {
         if (!target) return;
         TList objTList;
         // Cannot do better than this
         for (auto obj : objs) {
            if (obj && obj != target) objTList.Add(obj.get());
         }
         target->Merge(&objTList);
      }
   } // end of namespace TThreadedObjectUtils

   /**
    * \class ROOT::TThreadedObject
    * \brief A wrapper to make object instances thread private, lazily.
    * \tparam T Class of the object to be made thread private (e.g. TH1F)
    * \ingroup Multicore
    *
    * A wrapper which makes objects thread private. The methods of the underlying
    * object can be invoked via the the arrow operator. The object is created in
    * a specific thread lazily, i.e. upon invocation of one of its methods.
    * The correct object pointer from within a particular thread can be accessed
    * with the overloaded arrow operator or with the Get method.
    * In case an elaborate thread management is in place, e.g. in presence of
    * stream of operations or "processing slots", it is also possible to
    * manually select the correct object pointer explicitly.
    * The default size of the threaded objects is 64. This size can be extended
    * manually via the fgMaxSlots parameter. The size of individual instances
    * is automatically extended if the size of the implicit MT pool is bigger
    * than 64.
    *
    */
   template<class T>
   class TThreadedObject {
   public:
      static unsigned fgMaxSlots; ///< The maximum number of processing slots (distinct threads) which the instances can manage
      TThreadedObject(const TThreadedObject&) = delete;
      /// Construct the TThreaded object and the "model" of the thread private
      /// objects.
      /// \tparam ARGS Arguments of the constructor of T
      template<class ...ARGS>
      TThreadedObject(ARGS&&... args)
      {
         const auto imtPoolSize = ROOT::GetImplicitMTPoolSize();
         fMaxSlots = (64 > imtPoolSize) ? fgMaxSlots : imtPoolSize;
         fObjPointers = std::vector<std::shared_ptr<T>>(fMaxSlots, nullptr);
         fDirectories = Internal::TThreadedObjectUtils::DirCreator<T>::Create(fMaxSlots);

         TDirectory::TContext ctxt(fDirectories[0]);
         fModel.reset(Internal::TThreadedObjectUtils::Detacher<T>::Detach(new T(std::forward<ARGS>(args)...)));
      }

      /// Access a particular processing slot. This
      /// method is *thread-unsafe*: it cannot be invoked from two different
      /// threads with the same argument.
      std::shared_ptr<T> GetAtSlot(unsigned i)
      {
         if ( i >= fObjPointers.size()) {
            Warning("TThreadedObject::GetAtSlot", "Maximum number of slots reached.");
            return nullptr;
         }
         auto objPointer = fObjPointers[i];
         if (!objPointer) {
            objPointer.reset(Internal::TThreadedObjectUtils::Cloner<T>::Clone(fModel.get(), fDirectories[i]));
            fObjPointers[i] = objPointer;
         }
         return objPointer;
      }

      /// Set the value of a particular slot.
      void SetAtSlot(unsigned i, std::shared_ptr<T> v)
      {
         fObjPointers[i] = v;
      }

      /// Access a particular slot which corresponds to a single thread.
      /// This is in general faster than the GetAtSlot method but it is
      /// responsibility of the caller to make sure that an object is
      /// initialised for the particular slot.
      std::shared_ptr<T> GetAtSlotUnchecked(unsigned i) const
      {
         return fObjPointers[i];
      }

      /// Access a particular slot which corresponds to a single thread.
      /// This overload is faster than the GetAtSlotUnchecked method but
      /// the caller is responsible to make sure that an object is
      /// initialised for the particular slot and that the returned pointer
      /// will not outlive the TThreadedObject that returned it.
      T* GetAtSlotRaw(unsigned i) const
      {
         return fObjPointers[i].get();
      }

      /// Access the pointer corresponding to the current slot. This method is
      /// not adequate for being called inside tight loops as it implies a
      /// lookup in a mapping between the threadIDs and the slot indices.
      /// A good practice consists in copying the pointer onto the stack and
      /// proceed with the loop as shown in this work item (psudo-code) which
      /// will be sent to different threads:
      /// ~~~{.cpp}
      /// auto workItem = [](){
      ///    auto objPtr = tthreadedObject.Get();
      ///    for (auto i : ROOT::TSeqI(1000)) {
      ///       // tthreadedObject->FastMethod(i); // don't do this! Inefficient!
      ///       objPtr->FastMethod(i);
      ///    }
      /// }
      /// ~~~
      std::shared_ptr<T> Get()
      {
         return GetAtSlot(GetThisSlotNumber());
      }

      /// Access the wrapped object and allow to call its methods.
      T *operator->()
      {
         return Get().get();
      }

      /// Merge all the thread private objects. Can be called once: it does not
      /// create any new object but destroys the present bookkeping collapsing
      /// all objects into the one at slot 0.
      std::shared_ptr<T> Merge(TThreadedObjectUtils::MergeFunctionType<T> mergeFunction = TThreadedObjectUtils::MergeTObjects<T>)
      {
         // We do not return if we already merged.
         if (fIsMerged) {
            Warning("TThreadedObject::Merge", "This object was already merged. Returning the previous result.");
            return fObjPointers[0];
         }
         mergeFunction(fObjPointers[0], fObjPointers);
         fIsMerged = true;
         return fObjPointers[0];
      }

      /// Merge all the thread private objects. Can be called many times. It
      /// does create a new instance of class T to represent the "Sum" object.
      /// This method is not thread safe: correct or acceptable behaviours
      /// depend on the nature of T and of the merging function.
      std::unique_ptr<T> SnapshotMerge(TThreadedObjectUtils::MergeFunctionType<T> mergeFunction = TThreadedObjectUtils::MergeTObjects<T>)
      {
         if (fIsMerged) {
            Warning("TThreadedObject::SnapshotMerge", "This object was already merged. Returning the previous result.");
            return std::unique_ptr<T>(Internal::TThreadedObjectUtils::Cloner<T>::Clone(fObjPointers[0].get()));
         }
         auto targetPtr = Internal::TThreadedObjectUtils::Cloner<T>::Clone(fModel.get());
         std::shared_ptr<T> targetPtrShared(targetPtr, [](T *) {});
         mergeFunction(targetPtrShared, fObjPointers);
         return std::unique_ptr<T>(targetPtr);
      }

   private:
      unsigned fMaxSlots;                                ///< The size of the instance
      std::unique_ptr<T> fModel;                         ///< Use to store a "model" of the object
      std::vector<std::shared_ptr<T>> fObjPointers;      ///< A pointer per thread is kept.
      std::vector<TDirectory*> fDirectories;             ///< A TDirectory per thread is kept.
      std::map<std::thread::id, unsigned> fThrIDSlotMap; ///< A mapping between the thread IDs and the slots
      unsigned fCurrMaxSlotIndex = 0;                    ///< The maximum slot index
      bool fIsMerged = false;                            ///< Remember if the objects have been merged already
      ROOT::TSpinMutex fThrIDSlotMutex;                  ///< Mutex to protect the ID-slot map access

      /// Get the slot number for this threadID.
      unsigned GetThisSlotNumber()
      {
         const auto thisThreadID = std::this_thread::get_id();
         unsigned thisIndex;
         {
            std::lock_guard<ROOT::TSpinMutex> lg(fThrIDSlotMutex);
            auto thisSlotNumIt = fThrIDSlotMap.find(thisThreadID);
            if (thisSlotNumIt != fThrIDSlotMap.end()) return thisSlotNumIt->second;
            thisIndex = fCurrMaxSlotIndex++;
            fThrIDSlotMap[thisThreadID] = thisIndex;
         }
         return thisIndex;
      }

   };

   template<class T> unsigned TThreadedObject<T>::fgMaxSlots = 64;

} // End ROOT namespace

#include <sstream>

////////////////////////////////////////////////////////////////////////////////
/// Print a TThreadedObject at the prompt:

namespace cling {
   template<class T>
   std::string printValue(ROOT::TThreadedObject<T> *val)
   {
      auto model = ((std::unique_ptr<T>*)(val))->get();
      std::ostringstream ret;
      ret << "A wrapper to make object instances thread private, lazily. "
          << "The model which is replicated is " << printValue(model);
      return ret.str();
   }
}


#endif

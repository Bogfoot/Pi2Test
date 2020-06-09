/// \file RNTupleDS.hxx
/// \ingroup NTuple ROOT7
/// \author Jakob Blomer <jblomer@cern.ch>
/// \date 2018-10-04
/// \warning This is part of the ROOT 7 prototype! It will change without notice. It might trigger earthquakes. Feedback
/// is welcome!

/*************************************************************************
 * Copyright (C) 1995-2019, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#ifndef ROOT_RNTupleDS
#define ROOT_RNTupleDS

#include <ROOT/RDataFrame.hxx>
#include <ROOT/RDataSource.hxx>
#include <ROOT/RStringView.hxx>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ROOT {
namespace Experimental {

class RNTupleReader;
class REntry;


class RNTupleDS final : public ROOT::RDF::RDataSource {
   std::unique_ptr<ROOT::Experimental::RNTupleReader> fNTuple;
   std::unique_ptr<ROOT::Experimental::REntry> fEntry;
   unsigned fNSlots;
   bool fHasSeenAllRanges;
   std::vector<std::string> fColumnNames;
   std::vector<std::string> fColumnTypes;
   std::vector<void*> fValuePtrs;

public:
   RNTupleDS(std::unique_ptr<ROOT::Experimental::RNTupleReader> ntuple);
   ~RNTupleDS();
   void SetNSlots(unsigned int nSlots) final;
   const std::vector<std::string> &GetColumnNames() const final;
   bool HasColumn(std::string_view colName) const final;
   std::string GetTypeName(std::string_view colName) const final;
   std::vector<std::pair<ULong64_t, ULong64_t>> GetEntryRanges() final;

   bool SetEntry(unsigned int slot, ULong64_t entry) final;

   void Initialise() final;

protected:
   Record_t GetColumnReadersImpl(std::string_view name, const std::type_info &) final;
};

RDataFrame MakeNTupleDataFrame(std::string_view ntupleName, std::string_view fileName);

} // ns Experimental
} // ns ROOT

#endif

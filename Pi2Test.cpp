// Pi2Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#include <math.h>
#include <time.h>

using namespace std;

/*
	Treba napraviti MCSim za generiranje broja \pi. 
	U kvadratu površine 1 (1x1) generirati će se parovi brojeva (x,y)&[0-1].
	Upisati kružnicu radiusa 1, površine 1 * 1 * \pi= \pi. Prebrojati koliko se točaka nalazi u kružnici.
	BrTuKrug/BrTuKvad=\pi/4
*/
int main()
{
	srand(time(NULL));
	double rand_x, rand_y;
	double polozaj;
	int BrTuKrug = 0, BrTuKvad=100000;
	

	for (int i = 0; i < BrTuKvad; i++)
	{
		rand_x = ((double)rand() / (RAND_MAX));
		rand_y = ((double)rand() / (RAND_MAX));
		polozaj = pow(rand_x, 2) + pow(rand_y, 2);
		if (sqrt(polozaj) <= 1)
			BrTuKrug++;
		/*cout << "rand_x= " << rand_x <<"rand_y= "<< rand_y<< endl;*/
	}

	cout << BrTuKrug << endl;
	double pi = ((double)BrTuKrug / (double)BrTuKvad)*4;
	cout << "4*BrTuKrug/BrTuKvad:  " << pi << endl;
}

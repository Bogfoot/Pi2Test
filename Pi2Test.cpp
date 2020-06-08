// Pi2Test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <random>
#include <math.h>
#include <time.h>
#include <iomanip>


using namespace std;

/*
	Treba napraviti MCSim za generiranje broja \pi. 
	U kvadratu površine 1 (1x1) generirati će se parovi brojeva (x,y)&[0-1].
	Upisati kružnicu radiusa 1, površine 1 * 1 * \pi= \pi. Prebrojati koliko se točaka nalazi u kružnici.
	BrTuKrug/BrTuKvad=\pi/4
*/
int main()
{
	cout << std::fixed;
	cout << std::setprecision(8);
	srand(time(NULL));
	double rand_x, rand_y, BrPi[11];
	double polozaj;
	int n;
	char odg;
	cout << "Unesi potenciju: " << endl;
	cin >> n;

	for(int j =0; j<n;j++){
		int BrTuKrug = 0;
		for (int i = 0; i < pow(10,j); i++)
			{
				rand_x = ((double)rand() / (RAND_MAX));
				rand_y = ((double)rand() / (RAND_MAX));
				polozaj = pow(rand_x, 2) + pow(rand_y, 2);
				if (sqrt(polozaj) <= 1)
					BrTuKrug++;
				
			}
		double pi = ((double)BrTuKrug / pow(10, j) * 4);
			BrPi[j] = pi;
			cout << j + 1 << "-ti \pi je: " << BrPi[j] << endl;
	}
	cout << "Zelite li ispisati ponovno sve?(y/n)" << endl;
	cin >> odg;
	if (odg == 'y')
	{
		cout << "Skup \pi-jeva dobijen pomocu Monte Carlo metode." << endl;
		for (int i = 0; i < n; i++)
			cout << i + 1 << "-ti \pi je: " << BrPi[i] << endl;
	}
	else
	

	system("PAUSE");
	return 0;
}

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
	cout << std::setprecision(5);
	srand(time(NULL));
	double rand_x, rand_y, BrPi[11][11];
	double polozaj;
	int n;
	char odg, odg1;
	double srVrij[11] = {0.0}, stDev[11] = { 0.0 };

	 skok:
	cout << "Unesi potenciju: " << endl;
	cin >> n;

	for (int k = 0; k < n; k++) {
		for (int j = 0; j < n; j++) {
			int BrTuKrug = 0;
			for (int i = 0; i < pow(10, j); i++)
			{
				rand_x = ((double)rand() / (RAND_MAX));
				rand_y = ((double)rand() / (RAND_MAX));
				polozaj = pow(rand_x, 2) + pow(rand_y, 2);
				if (sqrt(polozaj) <= 1)
					BrTuKrug++;

			}
			double pi = ((double)BrTuKrug / pow(10, j) * 4);
			BrPi[k][j] = pi;
		}
	}
	cout << "Zelite li ispisati dobivene pi-jeve?(y/n)" << endl;
	cin >> odg;
	if (odg == 'y')
	{
		cout << "Skup \pi-jeva dobijen pomocu Monte Carlo metode." << endl;
		for (int i = 0; i < n; i++)
		{
			for (int j = 0; j < n; j++)
			{
				cout << "(" << i + 1 << ", " << j + 1 << ")-ti \pi je: " << BrPi[i][j] << " ";

			}
			cout << endl;
		}
	}
	else
	{
		
	}
		/*Treba skužit kako uključiti grafove */

	/*Srednja vrijednost i standardna devijacija*/
	

	cout << "Ovdje su srednje vrijednosti po identicnom eksperimentu." << endl;
	
	for (int i = 0; i < n; i++)
	{
		
			for (int j = 0; j < n; j++)
			{
				srVrij[i] += BrPi[j][i];
			}
			srVrij[i] = srVrij[i] / ((double)n);
	cout << "Srednja vrijednost za " << i + 1 << "-ti eksperiment je: " << srVrij[i] << endl;
	}
	
	cout << "Ovdje su standardne devijacije po identicnom eksperimentu." << endl;
	/*
		sqrt(suma(x_i - x_sr)^2/n)
	*/
	for (int i = 0; i < n; i++)
	{
		for(int j=0;j<n;j++)
		{
			stDev[i] += pow(BrPi[j][i] - srVrij[i], 2);
		}
		stDev[i] = sqrt(stDev[i] / ((double)n));
	}
	for(int i = 0; i < n; i++)
	{
		cout << "Standardne devijacije " << i + 1 << "-tog eksperimenta: " << stDev[i] << endl;
	}

	for (int i = 0; i < n; i++) 
	{
		cout << "Srednja vrijednost i standardna devijacija" << i + 1 << "-tog eksperimenta: " << srVrij[i] << " +- " << stDev[i] << endl;
	}

	cout << "Zelite li ponoviti sve eksperimente s drugom potencijom?(y/n)" <<endl ;
	cin >> odg1;
	if(odg1=='y')
		goto skok;
	else
	{
		goto stop;
	}
	stop:
	system("PAUSE");
	return 0;
}

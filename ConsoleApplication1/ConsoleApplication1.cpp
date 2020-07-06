// ConsoleApplication1.cpp: 定义控制台应用程序的入口点。
// index value is determined by
//  2 3 4  
//  1   5
//  0 7 6

#include <stdio.h>
#include <windows.h>
#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <graphics.h>
#include <conio.h>
#include <iostream>

// qqView.h : interface of the CQqView class
//
/////////////////////////////////////////////////////////////////////////////
//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000
#define name2str(name) (#name)

#define grid		100
#define THETA_FARMER	100.0 
#define THETA_HUNTER	100.0 
#define DISCOUNT		1.1

using namespace std;

class strategy
{
public:

	double getTheta(int row, int col) { return Theta[row][col]; }
	void setTheta(int row, int col, double x) { Theta[row][col] = x; }

private:
	
	double Theta[grid][grid];  // discount factor
};

class EvoProcess
{
public:
	void competition(void); 
	void evolution(void);
	void bargain(int row, int col, double sequence[]);
	double nash( double Theta1, double Theta2, double index);
	void update(int row, int col, double& Theta);
	~EvoProcess() { delete STR; }
	strategy* STR;

	int Lattice[grid*grid];
	double AverageValue[grid*grid];
};


 static COLORREF rgblist[2]{ RGB(255,255,0),RGB(255,15,0) };
// !defined(AFX_QQVIEW_H__A63D46DC_745B_4AB2_BC4F_EEE2662BCB25__INCLUDED_)


int main()
{
	EvoProcess* Evo = new EvoProcess();
	Evo->STR = new strategy();

	FILE *fp1;
	errno_t err = fopen_s(&fp1, "test.txt", "w");
	if (err == 0)
		printf("Test.txt opened\n");
	else
		printf("Test.txt not opened\n");

	double ratio[2] = { 0 };
	double AverTheta[2] = { 0 };
	double AverPayoff[2] = { 0 };
	double Error[2] = { 0 };

	int i, j;
			fflush(stdin);
			cout << "A market game: bargaining in a grid" << endl;
			cout << "enter initial percentage of farmers: ";

	srand((unsigned int)time(NULL));
	double value;

	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
		{
			value = (rand() % 100) / THETA_FARMER;
			if (value < 0.01) 
				value = 0.01;
			Evo->STR->setTheta(i,j,value);

			Evo->AverageValue[grid * i + j] = 0.0;
//			printf("%.2f ", Evo->STR->getTheta(i, j));
		}
	}

	initgraph(800, 1000, SHOWCONSOLE);
	int numsts = 0;
	for (i = 0; i < 2; i++) {
		if (ratio[i] != 0) {
			setfillcolor(rgblist[i]);
			solidrectangle(60 * numsts + 50, 850, 60 * numsts + 100, 900);
			switch (i) {
			case 0:  outtextxy(60 * numsts + 50, 910, _T("FARMER")); break;
			case 1: outtextxy(60 * numsts + 50, 910, _T("HUNTER")); break;
			}
			numsts++;
		}
	}
	
	int generation = 1;
//	printf("G-------%f----------%f \n", ratio[0], ratio[1]);

	do {
		fflush(stdin);
		for (i = 0; i < grid; i++)
		{
			for (j = 0; j < grid; j++)
			{
				setfillcolor(rgblist[Evo->Lattice[grid * i + j]]);
				solidrectangle(4 * j, 4 * i, 4 * j + 2, 4 * i + 2);
			}
		}

		AverTheta[0] = 0;
		AverTheta[1] = 0;
		AverPayoff[0] = 0;
		AverPayoff[1] = 0;

		for (i = 0; i < grid; i++)
		{
			for (j = 0; j < grid; j++)
			{
				AverTheta[0] += Evo->STR->getTheta(i, j);
				AverPayoff[0] += Evo->AverageValue[i*grid + j];
			}
		}
		AverTheta[0] /= grid*grid;	
		AverPayoff[0] /= grid*grid;
		
		
		cout << "Generation " << generation << ": "  <<AverPayoff[0] << endl;
		cout << "Average Theta " << ": " << AverTheta[0] << endl;
		
		Error[0] = 0;
		Error[1] = 0;
		for (i = 0; i < grid; i++)
		{
			for (j = 0; j < grid; j++)
					
					Error[0] += (Evo->AverageValue[i*grid + j] - AverPayoff[0])*(Evo->AverageValue[i*grid + j] - AverPayoff[0]);
				
		}

		Error[0] = sqrt(Error[0])/grid;
		cout << "Square error: "  << Error[0]  << endl;
		
/*
		for (i = 0; i < grid; i++)
		{
			for (j = 0; j < grid; j++)
				printf("%d ", Evo->STR->getID(i, j));
			printf("\n");
		}
		printf("\n\n");

		for (i = 0; i < grid; i++)
		{
			for (j = 0; j < grid; j++)
				printf("%.2f ", Evo->STR->getTheta(i, j));
			printf("\n");
		}
		printf("\n\n");
*/

		Evo->competition();		
		Evo->evolution();
		
		for (i = 0; i < grid; i++)
			for(j = 0; j < grid; j++)
				Evo->Lattice[grid * i + j] = (int)(2.0*Evo->STR->getTheta(i,j));

		generation++;
		
	} while (generation<100);

	fflush(stdin);
	closegraph();

	delete(Evo);
//	system("pause");
	fclose(fp1);
	return 0;
}

// create a sequence which lists Theta in increasing order 
// the agent with the smallest Theta has priority to choose the bargainer
// index[] stores the id of agents with Theta in increasing order
void EvoProcess::competition()
{
	int index[grid*grid][2];
	double sequence[grid*grid];

	int i, j;

	for (i = 0; i < grid; i++)
		for (j = 0; j < grid; j++)
		{
			sequence[i*grid+j] = STR->getTheta(i,j);
		}

		double min = 1;
		for (int k = 0; k < grid*grid; k++)
		{
			min = 1;
			for (i = 0; i < grid; i++)
			{
				for (j = 0; j < grid; j++)
				{
					if (sequence[i*grid + j] <= min)
					{
						min = sequence[i*grid + j];
						index[k][0] = i;
						index[k][1] = j;
					}
				}
			}
			sequence[index[k][0] * grid + index[k][1]] = 1.1;
		}
/*		printf("%d,%d\n",index[0][0],index[0][1]);
		printf("%.2f\n", STR->getTheta(index[0][0], index[0][1]));
		printf("%d,%d\n", index[9001][0], index[9001][1]);
		printf("%.2f\n", STR->getTheta(index[9001][0], index[9001][1]));
*/


		/*
		for (i = 0; i < grid*grid; i++)
			printf("%d %d\n", index[i][0],index[i][1]);
		printf("\n\n");

*/
	for (i = 0; i < grid; i++)
		for (j = 0; j < grid; j++)
			sequence[i*grid+j] = STR->getTheta(i, j);
/*
		for (i = 0; i < grid*grid; i++)
			printf("%0.2f\n", sequence[i]);
		printf("\n\n");
*/


	for (i = 0; i < grid*grid; i++)
	{
		if (sequence[index[i][0]*grid + index[i][1]] <= 1)
			bargain(index[i][0], index[i][1], sequence);
	}
	
/*	
	for (i = 0; i < grid*grid; i++)
		cout << sequence[i] << "  ";
	cout << endl;
	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
			cout << AverageValue[i*grid + j] << "  ";

		cout << endl;
	}
*/
	
	return;
}

// AverageValue[] stores the updated payoff of agents on the lattice
void EvoProcess::bargain(int row, int col, double sequence[] )
{
	double Theta1 = STR->getTheta(row, col);
	double payoff[8] = {0};

	if (row == 0 && col == 0)
	{
		payoff[5] = nash( Theta1, STR->getTheta(0,1), sequence[1]);
		payoff[6] = nash( Theta1, STR->getTheta(1, 1), sequence[grid+1]);
		payoff[7] = nash( Theta1, STR->getTheta(1, 0), sequence[grid]);
	}
	else if (row == 0 && col == grid - 1)
	{
		payoff[1] = nash( Theta1, STR->getTheta(0, grid-2), sequence[grid-2]);
		payoff[0] = nash( Theta1, STR->getTheta(1, grid - 2), sequence[grid+grid-2]);
		payoff[7] = nash( Theta1, STR->getTheta(1, grid - 1), sequence[grid+grid-1]);

	}
	else if (row == grid -1 && col == 0)
	{
		payoff[3] = nash( Theta1, STR->getTheta(grid - 2,0), sequence[grid*(grid-2)]);
		payoff[4] = nash( Theta1, STR->getTheta(grid - 2, 1), sequence[grid*(grid - 2)+1]);
		payoff[5] = nash( Theta1, STR->getTheta(grid - 1, 1), sequence[grid*(grid - 1)+1]);

	}
	else if (row == grid - 1 && col == grid - 1)
	{
		payoff[1] = nash( Theta1, STR->getTheta(grid - 1, grid - 2), sequence[grid*(grid - 1)+grid - 2]);
		payoff[2] = nash( Theta1, STR->getTheta(grid - 2, grid - 2), sequence[grid*(grid - 2) + grid - 2]);
		payoff[3] = nash( Theta1, STR->getTheta(grid - 2, grid - 1), sequence[grid*(grid - 2) + grid - 1]);

	}
	else if (row == 0)
	{
		payoff[0] = nash( Theta1, STR->getTheta(1, col-1), sequence[grid + col - 1]);
		payoff[1] = nash( Theta1, STR->getTheta(0, col - 1), sequence[col - 1]);
		payoff[5] = nash( Theta1, STR->getTheta(0, col + 1), sequence[col + 1]);
		payoff[6] = nash( Theta1, STR->getTheta(1, col + 1), sequence[grid + col + 1]);
		payoff[7] = nash( Theta1, STR->getTheta(1, col ), sequence[grid + col]);

	}
	else if (row == grid-1)
	{
		payoff[1] = nash( Theta1, STR->getTheta(row, col - 1), sequence[grid * row + col - 1]);
		payoff[2] = nash( Theta1, STR->getTheta(row -1, col - 1), sequence[grid * (row-1) + col - 1]);
		payoff[3] = nash( Theta1, STR->getTheta(row - 1, col), sequence[grid * (row-1) + col]);
		payoff[4] = nash( Theta1, STR->getTheta(row - 1, col +1), sequence[grid * (row-1) + col + 1]);
		payoff[5] = nash( Theta1, STR->getTheta(row, col + 1), sequence[grid * row + col + 1]);

	}
	else if (col == 0)
	{
		payoff[3] = nash( Theta1, STR->getTheta(row - 1, 0), sequence[grid * (row - 1)]);
		payoff[4] = nash( Theta1, STR->getTheta(row - 1, 1), sequence[grid * (row - 1) + 1]);
		payoff[5] = nash( Theta1, STR->getTheta(row, 1), sequence[grid * row + 1]);
		payoff[6] = nash( Theta1, STR->getTheta(row + 1, 1), sequence[grid * (row + 1) + 1]);
		payoff[7] = nash( Theta1, STR->getTheta(row+1, 0), sequence[grid * (row + 1)]);

	}
	else if (col == grid - 1)
	{
		payoff[0] = nash( Theta1, STR->getTheta(row + 1, col - 1), sequence[grid * (row + 1) + col -1]);
		payoff[1] = nash( Theta1, STR->getTheta(row , col - 1), sequence[grid * row + col - 1]);
		payoff[2] = nash( Theta1, STR->getTheta(row - 1, col - 1), sequence[grid * (row - 1) + col - 1]);
		payoff[3] = nash( Theta1, STR->getTheta(row - 1, col), sequence[grid * (row - 1) + col]);
		payoff[7] = nash( Theta1, STR->getTheta(row + 1, col), sequence[grid * (row + 1) + col]);

	}
	else
	{
		payoff[0] = nash( Theta1, STR->getTheta(row + 1, col - 1), sequence[grid * (row + 1) + col - 1]);
		payoff[1] = nash( Theta1, STR->getTheta(row, col - 1), sequence[grid * row + col - 1]);
		payoff[2] = nash( Theta1, STR->getTheta(row - 1, col - 1), sequence[grid * (row - 1) + col - 1]);
		payoff[3] = nash( Theta1, STR->getTheta(row - 1, col), sequence[grid * (row - 1) + col]);
		payoff[4] = nash( Theta1, STR->getTheta(row - 1, col + 1), sequence[grid * (row - 1) + col + 1]);
		payoff[5] = nash( Theta1, STR->getTheta(row, col + 1), sequence[grid * row + col + 1]);
		payoff[6] = nash( Theta1, STR->getTheta(row + 1, col + 1), sequence[grid * (row + 1) + col + 1]);
		payoff[7] = nash( Theta1, STR->getTheta(row + 1, col), sequence[grid * (row + 1) + col]);

	}
	// An agent chooses the best offer in neighborhood area 
	double result = 0;
	int count = 10;
	for (int i = 0; i < 8; i++)
	{
		if (payoff[i] > result)
		{
			result = payoff[i];
			count = i;
		}
	}

	AverageValue[row*grid + col] = result;
	sequence[row*grid + col] = 1.1;

	int row1, col1;
	if (count == 0)
	{
		row1 = row + 1;
		col1 = col - 1;
	}
	else if (count == 1)
	{
		row1 = row;
		col1 = col - 1;
	}
	else if (count == 2)
	{
		row1 = row - 1;
		col1 = col - 1;
	}
	else if (count == 3)
	{
		row1 = row - 1;
		col1 = col;
	}
	else if (count == 4)
	{
		row1 = row - 1;
		col1 = col + 1;
	}

	else if (count == 5)
	{
		row1 = row;
		col1 = col + 1;
	}
	else if (count == 6)
	{
		row1 = row + 1;
		col1 = col + 1;
	}
	else if (count == 7)
	{
		row1 = row + 1;
		col1 = col;
	}
	else
		return;

	result = nash( STR->getTheta(row1, col1), STR->getTheta(row, col), 0.5);
	AverageValue[row1*grid + col1] = result;
	sequence[row1*grid + col1] = 1.1;

	return;
}

double EvoProcess::nash( double Theta1,  double Theta2, double index)
{
	if (Theta2 > 1)
		return 0;
	else if ((Theta1 + Theta2) == 0)
		return 0.5;
	else
		return (Theta1 / (Theta1 + Theta2));
	
}

void EvoProcess::evolution()
{
	int i, j;
	double Theta[grid][grid];

	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
		{
			Theta[i][j] = STR->getTheta(i, j);
			update(i,j,Theta[i][j]);
		}
	}
/*
	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
			cout << ID[i][j] << "  ";
		cout << endl;
	}
	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
			cout << Theta[i][j] << "  ";
		cout << endl;
	}
*/
/*
	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
			cout << AverageValue[i*grid + j] << "  ";
		cout << endl;
	}
	
	for (i = 0; i < grid; i++)
	{
		for (j = 0; j < grid; j++)
		{
			STR->setTheta(i, j, Theta[i][j]);
		}
	}
*/
	return;
}

void EvoProcess::update(int row, int col, double& Theta)
{
	double payoff[8] = {0};
	if (row == 0 && col == 0)
	{
		payoff[5] = AverageValue[row*grid+col+1];
		payoff[6] = AverageValue[(row + 1)*grid + col + 1];
		payoff[7] = AverageValue[(row+1)*grid + col ];
	}
	else if (row == 0 && col == grid - 1)
	{
		payoff[1] = AverageValue[row * grid + col - 1];
		payoff[0] = AverageValue[(row + 1)*grid + col - 1];
		payoff[7] = AverageValue[(row + 1)*grid + col];

	}
	else if (row == grid - 1 && col == 0)
	{
		payoff[3] = AverageValue[(row-1)*grid + col ];
		payoff[4] = AverageValue[(row-1)*grid + col + 1];
		payoff[5] = AverageValue[row*grid + col + 1];

	}
	else if (row == grid - 1 && col == grid - 1)
	{
		payoff[1] = AverageValue[row * grid + col - 1];
		payoff[2] = AverageValue[(row - 1)*grid + col];
		payoff[3] = AverageValue[(row - 1)*grid + col];

	}
	else if (row == 0)
	{
		payoff[0] = AverageValue[(row + 1)*grid + col - 1];
		payoff[1] = AverageValue[row * grid + col - 1];
		payoff[5] = AverageValue[row*grid + col + 1];
		payoff[6] = AverageValue[(row + 1)*grid + col + 1];
		payoff[7] = AverageValue[(row + 1)*grid + col];

	}
	else if (row == grid - 1)
	{
		payoff[1] = AverageValue[row * grid + col - 1];
		payoff[2] = AverageValue[(row - 1)*grid + col];
		payoff[3] = AverageValue[(row - 1)*grid + col];
		payoff[4] = AverageValue[(row - 1)*grid + col + 1];
		payoff[5] = AverageValue[row*grid + col + 1];

	}
	else if (col == 0)
	{
		payoff[3] = AverageValue[(row - 1)*grid + col];
		payoff[4] = AverageValue[(row - 1)*grid + col + 1];
		payoff[5] = AverageValue[row*grid + col + 1];
		payoff[6] = AverageValue[(row + 1)*grid + col + 1];
		payoff[7] = AverageValue[(row + 1)*grid + col];

	}
	else if (col == grid - 1)
	{
		payoff[0] = AverageValue[(row + 1)*grid + col - 1];
		payoff[1] = AverageValue[row * grid + col - 1];
		payoff[2] = AverageValue[(row - 1)*grid + col];
		payoff[3] = AverageValue[(row - 1)*grid + col];
		payoff[7] = AverageValue[(row + 1)*grid + col];

	}
	else
	{
		payoff[0] = AverageValue[(row + 1)*grid + col - 1];
		payoff[1] = AverageValue[row * grid + col - 1];
		payoff[2] = AverageValue[(row - 1)*grid + col];
		payoff[3] = AverageValue[(row - 1)*grid + col];
		payoff[4] = AverageValue[(row - 1)*grid + col + 1];
		payoff[5] = AverageValue[row*grid + col + 1];
		payoff[6] = AverageValue[(row + 1)*grid + col + 1];
		payoff[7] = AverageValue[(row + 1)*grid + col];

	}

	double result = AverageValue[row*grid+col];
	int count = 10;
	for (int i = 0; i < 8; i++)
	{
		if (payoff[i] > result)
		{
			result = payoff[i];
			count = i;
		}
	}

	int row1, col1;
	if (count == 0)
	{
		row1 = row + 1;
		col1 = col - 1;
	}
	else if (count == 1)
	{
		row1 = row;
		col1 = col - 1;
	}
	else if (count == 2)
	{
		row1 = row - 1;
		col1 = col - 1;
	}
	else if (count == 3)
	{
		row1 = row - 1;
		col1 = col;
	}
	else if (count == 4)
	{
		row1 = row - 1;
		col1 = col + 1;
	}

	else if (count == 5)
	{
		row1 = row;
		col1 = col + 1;
	}
	else if (count == 6)
	{
		row1 = row + 1;
		col1 = col + 1;
	}
	else if (count == 7)
	{
		row1 = row + 1;
		col1 = col;
	}
	else
	{
		return;
	}
	STR->setTheta(row, col, STR->getTheta(row1, col1)); 
	return;
}










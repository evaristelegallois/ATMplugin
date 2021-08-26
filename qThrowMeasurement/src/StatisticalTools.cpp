//##########################################################################
//#                                                                        #
//#                CLOUDCOMPARE PLUGIN: FracDense                          #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 of the License.               #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                COPYRIGHT: AMANDINE FRATANI, CEREGE                     #
//#                                                                        #
//##########################################################################

#include "StatisticalTools.h"
#include "LinearRegression.h"


#include <vector>
#include <math.h>
#include <QDebug>

using namespace std;

double average(const vector<double>& values)
{
	double taille = static_cast<double>(values.size());
	double somme(0);

	for (unsigned i = 0; i < taille; ++i)
	{
		somme += values[i];
	}

	return somme / taille;
}

double variance(const vector<double>& values)
{
	double taille = static_cast<double>(values.size());
	double somme(0);
	double av = average(values);

	for (unsigned i = 0; i < taille; ++i)
	{
		somme += (values[i] - av)*(values[i] - av);
	}

	return somme / taille;
}

double standard_deviation(const vector<double>& values)
{
	return sqrt(variance(values));
}

double covariance(const vector<double>& values_x, const vector<double>& values_y)
{
	double av_x = average(values_x);
	double av_y = average(values_y);
	double taille = static_cast<double>(values_x.size());
	double somme(0);

	for (unsigned i = 0; i < taille; ++i)
	{
		somme += (values_x[i] - av_x) * (values_y[i] - av_y);
	}

	return somme / taille;
}

double distance_comparison_slope(const LinearRegression& l1, const LinearRegression& l2)
{
	double a1 = l1.getSlope();
	double a2 = l2.getSlope();

	qDebug() << "a1, a2" << a1 << a2;

	double b1 = l1.getOrdinate();
	double b2 = l2.getOrdinate();

	double var_x1 = variance(l1.getValues_x());
	double var_x2 = variance(l2.getValues_x());

	//qDebug() << "var_x1, var_x2" << var_x1 << var_x2;

	vector<double> x1 = l1.getValues_x();
	vector<double> x2 = l2.getValues_x();

	vector<double> y1 = l1.getValues_y();
	vector<double> y2 = l2.getValues_y();

	double n1 = static_cast<double>(x1.size());
	double n2 = static_cast<double>(x2.size());

	double Sa1 = 0;
	double Sa2 = 0;

	for (int i = 0; i < n1; i++)
	{
		Sa1 += (pow(y1.at(i) - (abs(a1) * x1.at(i) + b1), 2));
	}

	for (int j = 0; j < n2; j++)
	{
		Sa2 += (pow(y2.at(j) - (abs(a2) * x2.at(j) + b2), 2));
	}

	Sa1 = Sa1 / ((n1 - 2) * var_x1);
	Sa2 = Sa2 / ((n2 - 2) * var_x2);

	qDebug() << "Sa1, Sa2" << Sa1 << Sa2;

	double t = abs(a1 - a2) / sqrt(Sa1 + Sa2);
	return t;
}
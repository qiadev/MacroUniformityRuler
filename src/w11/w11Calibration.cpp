/*
 *  w11Calibration.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 8/29/08.
 *
 */

#include "w11Calibration.h"
using namespace std;

#include "BSpline.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

w11Calibration::w11Calibration(void)
:
fBS(NULL,0,NULL,1,0,1)
{
	fSin= fSout= NULL;
}

w11Calibration::~w11Calibration(void)
{
	freeLuts();
}

void w11Calibration::freeLuts(void)
{
	if (fSin) delete[] fSin;
	if (fSout) delete[] fSout;
	fSin= NULL;
	fSout= NULL;
}



void w11Calibration::generate(std::list<double> xdata, std::list<double> ydata)
{
	double *x = new double[xdata.size()];
	long i;
	list<double>::iterator dit;
	for (dit=xdata.begin(), i=0; dit != xdata.end(); ++dit, i++) {
		x[i]= *dit;
	}
	
	double *y = new double[ydata.size()];
	for (dit=ydata.begin(), i=0; dit != ydata.end(); ++dit, i++) {
		y[i]= *dit;
	}
	int ndata= xdata.size();
	
	// print data being fitted
	for (int i=0;i<ndata;i++) {
	//	cout << i << "\t" << x[i] << "\t" << y[i] << endl;
	}
	
    int wl= 5;			// cutoff wavelength
	int nnodes= 4 * wl;	// at least 2 per node, best 4 or more
    int bc= BSplineBase<double>::BC_ZERO_SECOND;
    BSpline<double>::Debug(0);	// 9 for debug information
    //BSpline<double> spline(x, ndata, y, wl, bc,nnodes);
	if (!fBS.setDomain(x, ndata, wl, bc, nnodes)) {
		throw string("B-spline setDomain failed");
	}
	if (!fBS.solve(y)) {
		throw string("B-spline solve failed");
	}
	if (!fBS.ok()) {
		throw string("B-spline failed");
	}
	
	// domain is CIE Y
	double smin= 0.25;
	double smax= 100;
	double ds= 0.25;
	long n= (long) ((smax - smin) / ds + 1);
	fSin= new double[n+1];		// reserve (more than) enough space
	fSout= new double[n+1];		// reserve (more than) enough space
	double s;
	for (i=0, s=smin; s <= smax; s += ds, i++) {
		fSin[i]= s;
		fSout[i]= fBS.evaluate(s);
	}
	fNdata= i;
	
	delete[] x;
	delete[] y;
}

double w11Calibration::evaluate(const double x)
{
	return fBS.evaluate(x);
}



string w11Calibration::serialize(void)
{
	ostringstream ost;
	ost.setf(ios::left);
	
	ost << "w11 calibration version 1.0.0" << endl;
	list<double>::iterator dit, dit2;
	ost << "DataCount:" << fNdata << endl;
	for (long i= 0; i<fNdata; i++) {
		ost.precision(5);
		ost.width(7);
		ost << fSin[i] << "\t" << fSout[i] << endl;
	}
	
	return ost.str();
}

void w11Calibration::loadCalibrationFile(std::string filepath)
{
	ifstream cf(filepath.c_str(), istream::in);
#define NLINE 256
	char line[NLINE];
	
	cf.getline(line, NLINE);
	cf.getline(line, NLINE);
	long ndata;
	int nc= sscanf(line,"DataCount:%ld", &ndata);
	if (nc != 1) {
		throw string("Error reading calibration file");
	}
	freeLuts();
	fSin= new double[ndata];
	fSout= new double[ndata];
	double sin, sout;
	for (long i=0;i<ndata;i++) {
		cf.getline(line, NLINE);
		nc= sscanf(line, "%lf %lf", &sin, &sout);
		fSin[i]= sin;
		fSout[i]= sout;
	}
	fNdata= ndata;
	
	//cout << "Read calibration file:" << endl;
	for (long i=0;i<fNdata; i++) {
	//	cout << i << " \t" << fSin[i] << "\t " << fSout[i] << endl;
	}
}

int comparedouble(const void * a, const void * b);
int comparedouble(const void * a, const void * b)
{
	if ( *(double*)a > *(double*)b) return +1;
	//if ( *(double*)a < *(double*)b) return -1;
	return 0;
}

double w11Calibration::interpolateInvers(const double targetLs)
{	
	long ilo= 0;
	long ihi= fNdata-1;
	double v;
	if (targetLs < fSout[ilo]) {
		v= fSin[0];
		//throw "Interpolation failed: target L* too low";
	} else if (targetLs > fSout[ihi]) {
		v= fSin[ihi];
		//throw "Interpolation failed: target L* too high";
	} else {
		long im= 0;
		while (ihi - ilo > 1) {
			im= ilo + (ihi-ilo)/2;
			if (targetLs < fSout[im]) {
				ihi= im;
			} else {
				ilo= im;
			}
		}
		assert(ihi == ilo+1);
		assert(targetLs < fSout[ihi]);
		
		v= fSin[ilo] + (targetLs - fSout[ilo]) * (fSin[ihi] - fSin[ilo])/(fSout[ihi] - fSout[ilo]);
		//cout << "Interpolation of " << targetLs << " between " << ilo << "(" << fSout[ilo] << ") and " << ihi << "(" << fSout[ihi] << ") = " << v << endl;
	}
	return v;
}






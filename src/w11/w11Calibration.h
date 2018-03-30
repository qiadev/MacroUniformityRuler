/*
 *  w11Calibration.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 8/29/08.
 *
 */


#ifndef __w11_calibration__h__
#define __w11_calibration__h__

#include "BSpline.h"

#include <list>
#include <string>

class w11Calibration {
public:
	w11Calibration(void);
	~w11Calibration();
	void generate(std::list<double> xdata, std::list<double> ydata);
	std::string serialize(void);
	double evaluate(const double x);		// only used during generation - uses B-spline
	double interpolateInvers(const double targetLs);		// used for interpolation from calibration read from file. Uses LUT.
	void loadCalibrationFile(std::string filepath);
	
private:
	double *fSin;
	double *fSout;
	long fNdata;
	
	BSpline<double> fBS;
	
	void freeLuts(void);

};


#endif





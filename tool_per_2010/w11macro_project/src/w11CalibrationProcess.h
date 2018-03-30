/*
 *  w11CalibrationProcess.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 8/29/08.
 *
 */

#ifndef __calibration_process__
#define __calibration_process__

#include "w11Calibration.h"

#include <string>
#include <list>

class w11CalibrationProcess {
public:
	void addCalibrationData(const std::string cinFilePath,const std::string coutFilePath);
	void generateCalibration(void);
	void storeCalibrationData(std::string filepath);
	void loadCalibrationData(std::string filepath);
	inline double calibrate(const double x) {
		return fCalibration.interpolateInvers(x);
	};
	
	
private:
	std::list<double> fXdata;
	std::list<double> fYdata;
	w11Calibration	fCalibration;

};

#endif



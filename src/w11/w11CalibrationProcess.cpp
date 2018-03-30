/*
 *  w11CalibrationProcess.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 8/29/08.
 *
 */

#include "w11CalibrationProcess.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
using namespace std;


string trim(char *line) {
	string sline= string(line);
	sline.erase(sline.find_last_not_of(" \r\t\n")+1);
	return sline;
}


void w11CalibrationProcess::addCalibrationData(const std::string cinFilePath,const std::string coutFilePath)
{
	ifstream fxdata(cinFilePath.c_str(), istream::in);
	ifstream fydata(coutFilePath.c_str(), istream::in);
	
	if (!fxdata.good()) {
		cout << "Could not open CIN input file:" << cinFilePath << endl;
		throw string("Problem with file name.");
	}
	
	if (!fydata.good()) {
		cout << "Could not open COUT input file:" << coutFilePath << endl;
		throw string("Problem with file name.");
	}
	
#define NLINE 256
	char line[NLINE];
	
	//
	// x data
	//
	long k, col, row;
	double cinvalue;
	while (fxdata.good()) {
		fxdata.getline(line, NLINE);
		int nc= sscanf(line, "%ld%ld%ld%lf", &k, &col, &row, &cinvalue);
		if (nc < 4) break;
		fXdata.push_back(cinvalue);
		//cout << "CINDATA: " << fXdata.size() << " " << cinvalue << endl;
	}
	
	//
	// y data
	//
	// search for start of data:
	while (fydata.good()) {
		fydata.getline(line, NLINE);
		if (trim(line) == "BEGIN_DATA") {
			cout << "BEGIN_DATA found" << endl;
			break; // found data start
		}	
	}
	
	// read data:
	while (fydata.good()) {
		fydata.getline(line, NLINE);
		if (trim(line) == "END_DATA") {
			cout << "reached END_DATA" << endl;
			break; // data is over
		}
		int iline;
		double ciel, ciea, cieb, ciex, ciey, ciez;
		int nc= sscanf(line, "%d%lf%lf%lf%lf%lf%lf", &iline, &ciel, &ciea, &cieb, &ciex, &ciey, &ciez);
		if (nc < 7) break;
		fYdata.push_back(ciey);
		//cout << "COUTDATA: " << fYdata.size() << " " << ciex << "\t" << ciey << "\t" << ciez << endl;
	}
	
	cout << "Number of data points: CIN file:" << fXdata.size() << " by CIE file:" << fYdata.size() << endl;
	
	if (fXdata.size() != fYdata.size()) {
		throw string("Inconsistent calibration data count");
	}
	
#undef NLINE
}

void w11CalibrationProcess::generateCalibration(void)
{
	fCalibration.generate(fXdata, fYdata);
}

void w11CalibrationProcess::storeCalibrationData(std::string filepath)
{
	ofstream cfile(filepath.c_str(), ostream::out);
	
	cfile << fCalibration.serialize();
}

void w11CalibrationProcess::loadCalibrationData(std::string filepath)
{
	fCalibration.loadCalibrationFile(filepath);	
}




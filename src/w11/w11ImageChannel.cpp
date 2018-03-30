/*
 *  w11ImageChannel.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/11/07.
 *	Written for INCITS W1.1 public domain.
 *
 */

#include "w11ImageChannel.h"

#include <fstream>
using namespace std;

w11ImageChannel::w11ImageChannel(void) {
	fdx= fdy= 0;
}

double w11ImageChannel::width(void) const 
{
	return nx() * fdx;
}

double w11ImageChannel::height(void) const 
{
	return ny() * fdy;
}


void w11ImageChannel::setScale(const double dy, const double dx)
{
	fdy= dy;
	fdx= dx;
}

void w11ImageChannel::writeFile(std::string filepath) const
{
	ofstream outfile(filepath.c_str());
	
	outfile << "<name>" << fName << "</name>" << endl;
	outfile << "<description>" << fEncodedDescription << "</description>" << endl;
	outfile << "<sizey>" << ny() << "</sizey>" << endl;
	outfile << "<sizex>" << nx() << "</sizex>" << endl;
	outfile << "<dy>" << fdy << "</dy>" << endl;
	outfile << "<dx>" << fdx << "</dx>" << endl;
	dumpData(outfile);
}








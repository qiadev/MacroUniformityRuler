#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <vector>

#include "w11CalibrationTP.h"
#include "w11Utilities.h"
#include "w11defects.h"

w11TestPattern::w11TestPattern(void)
{
	fFocusRangeMin= 40;
	fFocusRangeMax= 60;
}

void w11TestPattern::addPatches(void)
{
	double pwidth= 8;       // patch width
	double pheight= 8;      // patch height
	const double leftMargin= 20; // mm
	const double topMargin= 20;
	const double spacer= 0.2;
	float v= 0, vstart=0, vend=0;
	float deltav= 0;
	const long ixmax= 22;
	const long iymax= 28;
	long iy,ix;
	
	_ixmax= ixmax;
	_iymax= iymax;
	
	// first row uses large steps, and covers Y=0 to 100:
	iy= 0;
	v= 0;
	vstart= 0;
	vend= 100;
	deltav= (float) ((vend - vstart - 0.0001) / (ixmax - 1));
	for (ix=0;ix<ixmax;ix++) {
		w11Patch<float> patch(leftMargin + ix * (pwidth + spacer),topMargin + iy * (pheight + spacer), pwidth, pheight, v);
		patches.push_back(patch);
		v += deltav;
		if (v > vend) {
			v= vstart;
		}
	}
	
	// subsequent rows use small steps, just for the FocusRange:
	vstart= fFocusRangeMin;
	vend= fFocusRangeMax;
	deltav= 0.4;
	v= vstart;
	for (iy=1;iy<iymax;iy++) {
		for (ix=0;ix<ixmax;ix++) {
			w11Patch<float> patch(leftMargin + ix * (pwidth + spacer),topMargin + iy * (pheight + spacer), pwidth, pheight, v);
			patches.push_back(patch);
			v += deltav;
			if (v > vend) {
				v= vstart;
			}
		}
	}
	
	_iymax++;
	// last row uses large steps, and covers Y=100 to 0:
	vstart= 100;
	vend= 0;
	v= vstart;
	deltav= (float)((vstart - vend - 0.0001) / (ixmax - 1));
	for (ix=0;ix<ixmax;ix++) {
		w11Patch<float> patch(leftMargin + ix * (pwidth + spacer),topMargin + iy * (pheight + spacer),pwidth,pheight,v);
		patches.push_back(patch);
		v -= deltav;
		if (v < vend) {
			v= vstart;
		}
	}
}

w11ImageChannelT<w11T16Bits>* w11TestPattern::paint(void)
{
	const long dpi= 300;
	long nwidth= static_cast<long>(floor(dpi * 8.5));
	//nwidth= (nwidth/32 + 1) * 32;    // required for image setter
	long nheight= static_cast<long>(floor(dpi * 11.0));
	//nheight= (nheight/32 + 1) * 32;
	double dp= 25.4 / dpi;
	w11ImageChannelT<w11T16Bits> *cpage= new w11ImageChannelT<w11T16Bits>(nheight, nwidth);
	cpage->setScale(dp,dp);
	w11T16Bits val= 0;
	
	// paint all white:
	val= 65535;
	for (long iy=0;iy < nheight; iy++) {
		for (long ix= 0;ix < nwidth; ix++) {
			(*cpage)(iy, ix)= val;
		}
	}
	
	//for (float x=0;x<100;x+=1) {
	//	cout << x << "\t" << inverseTrc(x) << endl;
	//}
	
	{
		// write CIN values in order measured by gretag:
		ofstream fout;
		string filepath= w11Utility::instance().calibrationInputDirectory();
		filepath= w11Utility::appendPathSegment(filepath, "w11_calibration_cin.data");
		fout.open(filepath.c_str());
				  
		vector<float> pval(patches.size());
		list<w11Patch<float> >::iterator pit;
		long k= 0;
		for (pit=patches.begin(); pit != patches.end(); ++pit) {
			pval[k++]= pit->value();
		}
		// 
		long igcol, igrow;	// cols and rows as used by gretag
		for (igcol=0; igcol < _iymax; igcol++) {
			for (igrow=0;igrow < _ixmax; igrow++) {
				k= _ixmax * igcol + _ixmax - igrow - 1;
				fout << k << "\t" << igcol << "\t" << igrow << "\t" << pval[k] << endl;
			}
		}
	}
	
	// paint each patch:
	list<w11Patch<float> >::iterator pit;
	double pcvt= 1.0 / dp;
	for (pit=patches.begin(); pit != patches.end(); ++pit) {
		val= cvtfloat(pit->value());
		long pleft= static_cast<long>(pcvt * pit->left() + 0.5);
		long ptop= static_cast<long>(pcvt * pit->top() + 0.5);
		long pwidth= static_cast<long>(pcvt * pit->width() + 0.5);
		long pheight= static_cast<long>(pcvt * pit->height() + 0.5);
		//cout << pleft << "\t" << ptop << "\t" << pit->value() << endl;
		for (long iy=ptop;iy <= ptop+pheight; iy++) {
			for (long ix= pleft;ix <= pleft + pwidth; ix++) {
				(*cpage)(iy, ix)= val;
			}
		}
	}
	
	return cpage;
}










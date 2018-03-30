/*
 *  w11profile.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 5/3/09.
 *  Written for INCITS W1.1 Macro-uniformity
 *
 */


#include <vector>
#include <list>
#include <string>
#include <fstream>
#include <iostream>
using namespace std;

#include "w11ImageChannelT.h"
#include "w11fft.h"



template <typename T>
class w11Profile {
public:
	w11Profile(void) {
		stepsize= 0;
	};
	
	~w11Profile() {};
	
	// extend the profile smoothly to index length specified (e.g. to obtain length = 2^N)
	w11Profile(const w11Profile& p, long extendedLength) {
		long L= p.size() - 1;
		long E= extendedLength - p.size();
		//assert(E < L);
		assert(E > 0);
		stepsize= p.stepSize();
		data.resize(extendedLength, 0);
		for (long i=0;i<=L;i++) {
			data[i]= p[i];
		}
		// extend as follows:
		// L < x < L+E: eh(x) := 2 * e(L) - e(2*L - x)
		// L < x < L+E < 2*L: el(x) := 2 * e(0) - e(L+E-x)
		// L < x < L+X:		q= (x-L)/E
		//					p(x) := (1-q)*eh(x) + q * el(x)
		double eL= data[L];
		double e0= data[0];
		for (long ix= L+1; ix <= L+E; ix++) {
			double q= ((double)ix-L)/E;
			long isource= 2*L-ix;
			if (isource < 0) isource= L;
			double eh= 2 * eL - data[isource];
			isource= L+E-ix;
			if (isource>L) isource= 0;
			double el= 2 * e0 - data[isource];
			data[ix]= (1-q)*eh + q*el;
		}
	};
	
	inline const T operator[](long n) const {		// why: if returning reference T& then values are wrong
		assert(n >= 0);
		assert(n < data.size());
		return data[n];
	};
	inline T& operator()(long n) {
		assert(n >= 0);
		assert(n < data.size());
		return data[n];
	};
	
	long size(void) const {
		return data.size();
	};
	
	double stepSize(void) const
	{
		return stepsize;
	};
	
	void fromImage(const w11ImageChannelT<T> &image,const bool horizontal=true) {
		double end;
		if (horizontal) {
			end= image.height();	// yes height! this is the vertical segment used for the horizontal profile
		} else {
			end= image.width();
		}
		fromImage(image, 0, end, horizontal);
	};
	
	// start and end are in the orthogonal direction of the profile;
	// the length of the profile will always be the given by the dimension of the image
	void fromImage(const w11ImageChannelT<T> &image,const double start, const double end,const bool horizontal=true) {
				
		data.clear();
		
		if (horizontal) {
			stepsize= image.dx();
			long pstart= w11min(w11max(0,start / image.dy()), image.ny());
			long pend= w11min(w11max(0,1 + 0.5 + end/image.dy()), image.ny());
			if (pend < pstart) pend= pstart;
			long n= pend-pstart;
			data.resize(image.nx(), 0);
			for (long ix=0; ix < image.nx(); ix++) {
				for (long iy=pstart;iy<pend; iy++) {
					data[ix] += image(ix,iy);
				}
			}
			for (long ix=0; ix < image.nx(); ix++) {
				data[ix] /= n;
			}
		} else {
			stepsize= image.dy();
			long pstart= w11min(w11max(0,start / image.dx()), image.nx());
			long pend= w11min(w11max(0,1 + 0.5 + end/image.dx()), image.nx());
			if (pend < pstart) pend= pstart;
			long n= pend-pstart;
			data.resize(image.ny(), 0);
			for (long iy=0; iy < image.ny(); iy++) {
				for (long ix=pstart;ix<pend; ix++) {
					data[iy] += image(ix,iy);
				}
			}
			for (long iy=0; iy < image.ny(); iy++) {
				data[iy] /= n;
			}
		}
		
		// convert from Y to L*
		for (long i=0;i<data.size(); i++) {
			data[i]= 116 * pow(data[i]/100, 1.0/3.0) - 16;
		}
	};
	
#ifdef DRR_TP // DRR debug only
#include "drr_tp_filter_csf.h"
#endif
	
	
	void writeProfile(string filepath) const {
		ofstream outfile(filepath.c_str());
		
		long i;
		for (i=0;i<data.size(); i++) {
			outfile << (i * stepsize) << "\t" << data[i] << endl;
		}	
	};
	
#ifdef DRR_TP // DRR debug only
#include "drr_tp_analyze.h"
#endif
	
	
private:
	vector<double> data;
	double stepsize;
	
	
};



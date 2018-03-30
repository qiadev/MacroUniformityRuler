#ifndef _w11_fft_h_
#define _w11_fft_h_

/*
 *  w11fft.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 5/1/08.
 *  Written for INCITS W1.1 Macro-uniformity
 *
 */

#include "w11Matrix.h"
#include "w11complex.h"
#include <string>
#include <iostream>
#include <complex>
#include <assert.h>


class w11FFTBase {
public:
	static long validatePowerOf2(const long n);
	static long* bitReversal(const long n);
	static std::string binrep(const short int n, const short int nbase = 0);
	
};

/*
inline std::complex<double> operator*(std::complex<double> x, std::complex<float> y)
{
	std::complex<double> z;
	z.real() = x.real() * y.real() - x.imag() * y.imag();
	z.imag() = x.real() * y.imag() + x.imag() * y.real();
	return z;
}

inline std::complex<double> operator*(std::complex<float> x, std::complex<double> y)
{
	std::complex<double> z;
	z.real() = x.real() * y.real() - x.imag() * y.imag();
	z.imag() = x.real() * y.imag() + x.imag() * y.real();
	return z;
}

inline std::complex<double> operator+(std::complex<double> x, std::complex<float> y)
{
	std::complex<double> z(x.real() + y.real(), x.imag() + y.imag());
	return z;
}

inline std::complex<double> operator+(std::complex<float> x, std::complex<double> y)
{
	std::complex<double> z(x.real() + y.real(), x.imag() + y.imag());
	return z;
}

inline std::complex<double> operator-(std::complex<double> x, std::complex<float> y)
{
	std::complex<double> z(x.real() - y.real(), x.imag() - y.imag());
	return z;
}

inline std::complex<double> operator-(std::complex<float> x, std::complex<double> y)
{
	std::complex<double> z(x.real() - y.real(), x.imag() - y.imag());
	return z;
}
 */

template <typename T>
class w11FFT : w11FFTBase {
public:
	w11FFT(const long n)
	: fn(n), permutation(0)
	{
		fM= w11FFTBase::validatePowerOf2(fn);
		cdata= new w11complex<T>[fn];
		assert(cdata);
		permutation= w11FFTBase::bitReversal(fn);
	};
	
	~w11FFT()
	{
		if (cdata) {
			delete [] cdata;
		}
		if (permutation) {
			delete [] permutation;
		}
	};
	
	inline w11complex<T> operator()(long i) const {return cdata[i];};
	
	void transformRealData(const T* const data, bool forward=true) 
	{
		// copy into cdata using permutation
		for (long i=0;i<fn;i++) {
			cdata[i].real()= data[permutation[i]];
			cdata[i].imag()= 0;						// imaginary part
		}
		transform(forward);
		// check memory leak here
	};

	void transformComplexData(const w11complex<T>* const data, bool forward=true) 
	{
		// copy into cdata using permutation
		for (long i=0; i<fn; i++) {
			cdata[i] = data[permutation[i]];
		}
		transform(forward);
	};
	
	void getComplexData(w11complex<T>* buffer) const
	{
		w11complex<T> *pbuf= buffer;
		for (long i=0; i<fn; i++) {
			*pbuf++ = cdata[i];
		}
	};
	
	void getComplexDataBuffer(T* buffer) const
	{
		T* pbuf= buffer;
		for (long i=0; i<fn; i++) {
			*pbuf++ = cdata[i].real();
			*pbuf++ = cdata[i].imag();
		}
	};
	
	void getRealData(T* buffer) const
	{
		T* pbuf= buffer;
		for (long i=0; i<fn; i++) {
			*pbuf++ = cdata[i].real();
		}
	};
	
	void getImagData(T* buffer) const
	{
		T* pbuf= buffer;
		for (long i=0; i<fn; i++) {
			*pbuf++ = cdata[i].imag();
		}
	};
	
	
	void printData(void)
	{
		std::cout << "======================================================" << std::endl;
		std::cout << "N=" << fn << " M=" << fM << std::endl;
		for (long i=0; i<fn; i++) {
			std::cout << i << " " << cdata[i].real() << " " << cdata[i].imag() << std::endl;
		}
	};
	
	
private:
	long fn;
	long fM;					// such that fn == 2^fM
	w11complex<T>* cdata;
	long* permutation;
	
	void transform(bool forward)
	{
		// does complex operators work between <float> and <double>:
		w11complex<double> cdouble;
		w11complex<float> cfloat;
		w11complex<double> tdouble;
		tdouble = cdouble * cfloat;
		tdouble = cdouble + cfloat;
		tdouble = cfloat + cdouble;
		
		long dbit= 1;					// the dual-node pair bit being operated on
		const double twopi= 6.283185307179586;
		w11complex<double> W;
		w11complex<double> t0, t1;			// temp variables
		long a= 0, b= 0;
		w11complex<double> wpowa;				// W ^ a
		
		for (long pass= 1; pass <= fM; pass++) {
			long two_dbit= 2 * dbit;
			// W = exp(-2*pi*i / 2*dbit):
			// note: 2*sin^2(x/2) == 1 - cos(x)
			const double theta= twopi / two_dbit;
			W.real()= cos(theta);
			if (forward) {
				W.imag()= sin(theta);
			} else {
				W.imag()= -sin(theta);
			}
			//
			for (a=0; a < dbit; a++) {
				std::complex<double> Wc(W.real(), W.imag());
				std::complex<double> ztmp= pow(Wc, a);
				wpowa.real()= ztmp.real();
				wpowa.imag()= ztmp.imag();
				for (b=0; b < fn; b += two_dbit) {
					t0= cdata[b+a] + wpowa * cdata[b+dbit+a];
					t1= cdata[b+a] - wpowa * cdata[b+dbit+a];
					cdata[b+a]= t0;
					cdata[b+dbit+a]= t1;
				}
			}
			dbit *= 2;
		}
		
		// scale
		if (!forward) {
			for (long i=0; i<fn; i++) {
				cdata[i].real() /= fn;
				cdata[i].imag() /= fn;
			}
		}
	};
	
};

template <typename T>
class w11FFT2D : w11FFTBase {
public:
	w11FFT2D(const long ny, const long nx)
	: fny(ny), fnx(nx), cmatrix(ny, nx)
	{
		w11FFTBase::validatePowerOf2(fny);
		w11FFTBase::validatePowerOf2(fnx);

		//cmatrix= new w11Matrix<w11complex<T> >(ny, nx);
	};
	
	~w11FFT2D()
	{
		
	};
	
	void setRealMatrix(const w11Matrix<T>& m)		// copies real data into the complex matrix.
	{
		assert(m.sizey() == fny);
		assert(m.sizex() == fnx);
		
		//mref= m;
		for (long iy=0; iy < fny; iy++) {
			for (long ix=0; ix < fnx; ix++) {
				cmatrix[iy][ix].real()= m[iy][ix];
				cmatrix[iy][ix].imag()= 0;
			}
		}
	};
	
	void setComplexMatrix(const w11Matrix< w11complex<T> >& m)		// copies complex data into the complex matrix.
	{
		assert(m.sizey() == fny);
		assert(m.sizex() == fnx);
		
		for (long iy=0; iy < fny; iy++) {
			for (long ix=0; ix < fnx; ix++) {
				cmatrix[iy][ix]= m[iy][ix];
			}
		}
	};
	
	void transform(bool forward) 
	{
		w11FFT<T> xfft1d(fnx);
		
		// 1D transforms of rows:
		for (long iy=0; iy<fny; iy++) {
			xfft1d.transformComplexData(cmatrix[iy], forward);
			xfft1d.getComplexData(cmatrix[iy]);
		}
		
		// 1D transforms of columns:
		w11FFT<T> yfft1d(fny);
		w11complex<T> *col = new w11complex<T>[fny];
		for (long ix=0; ix<fnx; ix++) {			// loop over all columns
			// copy column data
			for (long iy=0; iy<fny; iy++) {
				col[iy]= cmatrix[iy][ix];
			}
			xfft1d.transformComplexData(col, forward);
			xfft1d.getComplexData(col);
			// copy transformed column back into matrix:
			for (long iy=0; iy<fny; iy++) {
				cmatrix[iy][ix]= col[iy];
			}
		}
		
		delete[] col;
	};
	
	void getRealComponent(w11Matrix<T> &m) const
	{
		for (long iy=0; iy<fny; iy++) {
			for (long ix=0; ix<fnx; ix++) {
				m[iy][ix]= cmatrix[iy][ix].real();
			}
		}
	};
	
	void getImagComponent(w11Matrix<T> &m) const
	{
		for (long iy=0; iy<fny; iy++) {
			for (long ix=0; ix<fnx; ix++) {
				m[iy][ix]= cmatrix[iy][ix].imag();
			}
		}
	};
	
	void getComplexMatrix(w11Matrix< w11complex<T> > &m) const
	{
		for (long iy=0; iy<fny; iy++) {
			for (long ix=0; ix<fnx; ix++) {
				m[iy][ix]= cmatrix[iy][ix];
			}
		}
	};
	
private:
	long fny;
	long fnx;
	//const w11Matrix<T> &mref;
	w11Matrix<w11complex<T> > cmatrix;
	

	
};

#endif

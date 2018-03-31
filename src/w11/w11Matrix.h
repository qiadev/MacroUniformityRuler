#ifndef _w11_matrix_h_
#define _w11_matrix_h_

/*
 *  w11Matrix.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 4/30/08.
 *
 */

#include "w11Types.h"

#include <math.h>
#include <ostream>


template <typename T>
class w11Matrix {
	
// MARK: -  Private data members
private:
	long fnx;		// columns
	long fny;		// rows
	T **v;			// values
	
// MARK: -  Public methods
public:
	w11Matrix(void)
	{
		fnx= 0;
		fny= 0;
		v= 0L;
	};
	
	
	~w11Matrix()
	{
		freedata();
	};
	
	explicit w11Matrix(long ny, long nx)				// matrix notation for indexing: (row, column) that is (y, x)
	: 
	fnx(nx), 
	fny(ny), 
	v(new T*[ny])					// pointers to rows
	{
		v[0]= new T[fny * fnx];		// allocate as one big chunk - needed for most matrix libraries
		//							// v[0] acts in two ways: point to entire block, and point to first row.
		for (long iy=1; iy<fny; iy++) {
			v[iy]= v[iy-1] + fnx;
		}
	};
	
	w11Matrix(const w11Matrix &rhs)
	:
	fnx(rhs.fnx),
	fny(rhs.fny),
	v(new T*[fny])
	{
		long ix, iy;
		v[0]= new T[fny * fnx];
		for (iy=1; iy<fny; iy++) {
			v[iy]= v[iy-1] + fnx;
		}
		T* sptr= 0, dptr= 0;
		for (iy=0; iy<fny; iy++) {
			dptr= v[iy];
			sptr= rhs[iy];
			for (ix=fnx; ix--;) {
				*dptr++ = *sptr++;
			}
		}
	};
	
	w11Matrix& operator=(const w11Matrix &rhs)
	{
		if (this == &rhs) {
			return *this;
		}
		
		long ix, iy;
		// resize if necessary:
		if (fnx != rhs.fnx || fny != rhs.fny) {
			freedata();
			fnx= rhs.fnx;
			fny= rhs.fny;
			v= new T*[fny];
			v[0]= new T[fnx * fny];
		}
		// now ready for copy:
		T* sptr= 0, dptr= 0;
		for (iy=0; iy<fny; iy++) {
			dptr= v[iy];
			sptr= rhs[iy];
			for (ix=fnx; ix--;) {
				*dptr++ = *sptr++;
			}
		}
		return *this;
	};
	
	w11Matrix& operator=(const T &a)
	{
		long idx;
		long n= fnx * fny;
		T* vptr= v[0];
		for (idx= n; idx--; ) {
			*vptr++ = a;
		}
		return *this;
	};
	
	w11Matrix& operator*=(const T& a)
	{
		long idx;
		long n= fnx * fny;
		T* vptr= v[0];
		for (idx= n; idx--; ) {
			*vptr++ *= a;
		}
		return *this;
	};
	
	
	inline T* operator[](const long i) {
		return v[i];
	};
	
	inline const T* operator[](const long i) const
	{
		return v[i];
	};
	
	inline long sizex() const
	{
		return fnx;
	};
	
	inline long sizey() const
	{
		return fny;
	};	
	
	void getRange(T &minimum, T &maximum) const		// assumes T is ordered
	{
		T vmin= 0;
		T vmax= 0;
		T* vptr= 0;
		
		vmin= vmax= static_cast<T>(double(v[0][0]));	// cast to avoid warning from MSVS about double to float data loss
		for (long iy=0;iy<fny;iy++) {
			vptr= v[iy];
			for (long ix=fnx;ix--;) {
				if (*vptr > vmax) {
					vmax= *vptr;
				} else if (*vptr < vmin) {
					vmin= *vptr;
				}
				vptr++;
			}
		}
		minimum= vmin;
		maximum= vmax;
	};
	
	void getStat1(double &average) const				// assumes conversion to double is possible
	{
		T* vptr= 0;
		double sum= 0;
		
		for (long iy=0;iy<fny;iy++) {
			vptr= v[iy];
			for (long ix=fnx;ix--;) {
				sum += double(*vptr++);
			}
		}
		average= sum / (fnx * fny);
	};
	
	void getStat1(double &average, double &rms) const	// assumes conversion to double is possible
	{
		T* vptr= 0;
		double sum= 0;
		double sumsqr= 0;
		double vd= 0;
		
		for (long iy=0;iy<fny;iy++) {
			vptr= v[iy];
			for (long ix=fnx;ix--;) {
				vd= double(*vptr++);
				sum += vd;
				sumsqr += vd*vd; 
			}
		}
		average= sum / (fnx * fny);
		double ds= sumsqr/(fnx*fny) - average*average;
		if (ds < 0) {
			rms= 0;
		} else {
			rms= sqrt(ds);
		}
	};
	
	void dumpData(std::ostream& ost) const
	{
		// v[0] is pointer to the entire data chunk
		size_t bsize= fnx * fny * sizeof(T);
		ost.write((char*)v[0], bsize);
	};
	
	
// MARK: -  Private methods
private:
	void freedata(void) {
		if (v) {
			delete[] (v[0]);	// delete the data memory block
			delete[] (v);		// deleta the pointers to the rows
			v= 0;
		}
	};
	
};

void writeTiff(const w11Matrix<w11T8Bits>& matrix, std::string filepath, float xdpi, float ydpi, const std::string& description);



#endif


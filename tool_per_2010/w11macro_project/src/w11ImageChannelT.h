#ifndef _w11_image_channel_t_h_
#define _w11_image_channel_t_h_
/*
 *  w11ImageChannelT.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 4/29/08.
 *	Written for INCITS W1.1 public domain.
 *
 *	Template for channels of specific pixel type (P)
 *
 */

#include <stdlib.h>

#include "w11Matrix.h"
#include "w11ImageChannel.h"
#include "w11Exceptions.h"

// P is the pixel type
template <typename P> class w11ImageChannelT : public w11ImageChannel {
#pragma mark Private data members	
private:   
	w11Matrix<P> m;

#pragma mark Public methods	
public:
	w11ImageChannelT(void)
	{
	};
	
	w11ImageChannelT(int pny, int pnx)
	: m(pny, pnx)
	{
	};
	
	~w11ImageChannelT()
	{
	};
	
	inline long nx(void) const {return m.sizex();};
	inline long ny(void) const {return m.sizey();};

	void fillWithTestContent(void)
	{
		int k= 0;
		for (int ix=0;ix<m.sizex();ix++) {
			int tval= k % 256;
			k++;
			for (int iy=0;iy<m.sizey();iy++) {
				//m[iy][jx]= tval;
				(*this)(iy, ix)= tval;
			}
		}
	};
	
	// image pixel access (get):
	w11Matrix<P>& matrix(void) {return m;};
	const P* constRowPtr(int py) const {return m[py];};
	P* rowPtr(int py) {return m[py];};

	// image pixel access (set):
	inline const P& operator()(int py, int px) const {return m[py][px];};
	inline P& operator()(int py, int px) {return m[py][px];};
	
	// statistics:
	void getRange(P &minimum, P &maximum) const
	{
		m.getRange(minimum, maximum);
	};
	
	void getStat1(double &average) const
	{
		m.getStat1(average);
	};
	
	void getStat1(double &average, double &rms) const
	{
		m.getStat1(average, rms);
	};	

	virtual void dumpData(std::ostream& ost) const
	{
		m.dumpData(ost);
	};
  
   
};


#endif


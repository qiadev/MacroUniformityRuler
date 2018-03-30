#ifndef _w11_calibration_tp_h
#define _w11_calibration_tp_h
/*
 *  w11CalibrationTP.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 8/29/08.
 *
 */

#include "w11ImageChannelT.h"
#include "w11Types.h"

#include <list>

/** @file
 *
 */

template<typename TPixelType> class w11Patch {
private:
	double _left;   /**< left edge [mm] */
	double _top;    /**< top edge [mm] */
	double _width;  /**< width [mm] */
	double _height; /**< height [mm] */
	
	TPixelType _value; /**< average value of patch */
	
public:
	w11Patch(const double l, const double t, const double w, double h, const TPixelType& v)
	{
		_left= l;
		_top= t;
		_width= w;
		_height= h;
		_value= v;
	}
	double left(void) const {
		return _left;
	}
	double top(void) const {
		return _top;
	}
	double width(void) const {
		return _width;
	}
	double height(void) const {
		return _height;
	}
	TPixelType value(void) const {
		return _value;
	}
	
};

class w11TestPattern {
private:
	std::list<w11Patch<float> > patches;
	long _iy;
	long _ix;
	long _ixmax;
	long _iymax;
	float fFocusRangeMin;
	float fFocusRangeMax;
	
public:
	w11TestPattern();
	void addPatches(void);
	w11ImageChannelT<w11T16Bits>* paint(void);   /**< XYZ space, range [0,100] */
};


#endif


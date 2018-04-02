#ifndef _w11_image_channel_h_
#define _w11_image_channel_h_

/*
 *  w11ImageChannel.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/11/07.
 *	Written for INCITS W1.1 public domain.
 *
 *	Defines ImageChannel interface.
 *
 */

#include <ostream>
#include <string>
#include <stdlib.h>

#include "w11Exceptions.h"

class w11ImageChannel {
public:
	
	w11ImageChannel(void);
	
	// size:
	virtual long nx(void) const = 0;
	virtual long ny(void) const = 0;
	double width(void) const;					// physical width
	double height(void) const;					// physical height
	
	// scale:
	inline double dx(void) const {return fdx;};
	inline double dy(void) const {return fdy;};
	void setScale(const double dx, const double dy);
	
	// IO:
    std::string  writeFile(std::string filepath) const;
	virtual void dumpData(std::ostream& ost) const = 0;
	
private:
	std::string fName;
	std::string fEncodedDescription;	// e.g. XML
	double fdx;
	double fdy;


};

#endif


#ifndef _w11_image_
#define _w11_image_

/*
 *  w11Image.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/11/07.
 *
 */
 
#include <vector>

#include "w11ImageChannel.h"


class w11Image {
public:
	long channelCount(void) const;
	w11ImageChannel& channel(long idx);



private:
	std::vector< w11ImageChannel* > fChannels;




};



#endif

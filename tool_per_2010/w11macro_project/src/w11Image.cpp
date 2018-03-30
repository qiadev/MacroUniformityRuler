/*
 *  w11Image.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/11/07.
 *
 */

#include "w11Image.h"

#include <assert.h>



long w11Image::channelCount(void) const
{
	return fChannels.size();
}

w11ImageChannel& w11Image::channel(long idx)
{
	assert(idx >= 0);
	assert(idx < fChannels.size());
	
	return *(fChannels[idx]);

}


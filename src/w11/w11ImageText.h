/*
 *  w11TpText.h
 *  w11macro
 *
 *	Add text to test pattern images
 *
 *  Created by Rene Rasmussen on 6/24/08.
 *
 */
#ifndef _w11_image_text_
#define _w11_image_text_

#include "w11Types.h"
#include "w11ImageChannelT.h"

#include <string>

#if _SUPPORT_FREETYPE_
struct FT_Bitmap_;
#else
typedef void* FT_Bitmap;
typedef void* FT_Bitmap_;
#endif


class w11ImageText {
private:
	std::string fText;
	double fPointSize;
	std::string fFontDirectory;
	std::string fFontName;
	
public:
	w11ImageText(std::string fontDirectory, std::string fontName="Arial");
	~w11ImageText();
	
	void setFontSize(double ptsize);
	void setFontName(std::string fontName);
	void setText(std::string t);
	
	void addTextAt(w11ImageChannelT<w11T8Bits>& image, double xmm, double ymm);

private:
	void draw_bitmap(w11ImageChannelT<w11T8Bits>& image, FT_Bitmap_* bitmap, long x,long y);

};


#endif


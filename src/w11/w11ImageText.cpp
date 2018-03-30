/*
 *  w11TpText.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 6/24/08.
 *
 */

#include "w11ImageText.h"
#include "w11Utilities.h"

#if _SUPPORT_FREETYPE_
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

using namespace std;



w11ImageText::w11ImageText(std::string fontDirectory, std::string fontName)
{
	fFontDirectory= fontDirectory;
	fFontName= fontName;
	fPointSize= 10;
}


w11ImageText::~w11ImageText()
{
	
}

void w11ImageText::setFontSize(double ptsize)
{
	fPointSize= ptsize;
}

void w11ImageText::setFontName(std::string fontName)
{
	fFontName= fontName;
}


void w11ImageText::setText(std::string t)
{
	fText= t;
}



void w11ImageText::addTextAt(w11ImageChannelT<w11T8Bits>& image, double xmm, double ymm)
{
#if _SUPPORT_FREETYPE_

	FT_Library    library;
	FT_Face       face;
	
	FT_GlyphSlot  slot;
	FT_Matrix     matrix;                 /* transformation matrix */
	FT_Vector     pen;                    /* untransformed origin  */
	FT_Error      error;
	
	string fontFileName= w11Utility::appendPathSegment(fFontDirectory, fFontName);
	
	double angle;
	double degrees= 0.0;
	int n, num_chars;
	
	num_chars= fText.length();
	angle= ( degrees / 360 ) * 3.14159 * 2;      /* use 25 degrees     */
	
	error= FT_Init_FreeType( &library );              /* initialize library */
	if (error) {
		throw "FT_Init_FreeType failed";
	}
	
	error= FT_New_Face( library, fontFileName.c_str(), 0, &face ); /* create face object */
	if (error) {
		throw "FT_New_Face failed";
	}
	
	long xdpi= (long) (25.4 / image.dx());
	long ydpi= (long) (25.4 / image.dy());
	long psize266= (long) (fPointSize * 72);
	
	error= FT_Set_Char_Size( face, psize266, 0, xdpi, ydpi );                /* set character size */
	if (error) {
		throw "FT_Set_Char_Size failed";
	}
	
	slot= face->glyph;
	
	/* set up matrix */
	matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );
	
	pen.x = 0;
	pen.y = 0;
	long xoffset= (long) (xmm / image.dx());
	long yoffset= (long) (ymm / image.dy());		// baseline position
	
	for ( n = 0; n < num_chars; n++ ) {
		FT_Set_Transform( face, &matrix, &pen );				//set transformation
		error= FT_Load_Char( face, fText[n], FT_LOAD_RENDER );	// load glyph image into the slot (erase previous one)
		if (error) {
			continue;											/* ignore errors */
		}
		
		/* now, draw to our target surface (convert position) */
		draw_bitmap(image, &slot->bitmap, xoffset + slot->bitmap_left, yoffset - slot->bitmap_top);
		
		/* increment pen position */
		pen.x += slot->advance.x;
		pen.y += slot->advance.y;
	}
	
	FT_Done_Face(face);
	FT_Done_FreeType(library);
#endif
}


void w11ImageText::draw_bitmap(w11ImageChannelT<w11T8Bits>& image, FT_Bitmap* bitmap, long x,long y)
{
#if _SUPPORT_FREETYPE_
	FT_Int  i, j, p, q;
	FT_Int  x_max = x + bitmap->width;
	FT_Int  y_max = y + bitmap->rows;
	
	
	for ( i = x, p = 0; i < x_max; i++, p++ ) {
		for ( j = y, q = 0; j < y_max; j++, q++ ) {
			if ( i < 0 || j < 0 || i >= image.nx() || j >= image.ny() ) {
				continue;
			}
			//image(j, i) |= bitmap->buffer[q * bitmap->width + p];
			//image(j, i)= 0;
			
			//image(j, i) = 255 - bitmap->buffer[q * bitmap->width + p];	// black text on white
			//image(j, i) = bitmap->buffer[q * bitmap->width + p];			// white text on black
			{ 
				// here we just paint the text that is pure 255
				// if some pixels are anti-aliased (gray) then they will be erased (not rendered)
				if (bitmap->buffer[q * bitmap->width + p] == 255) {
					image(j, i) = 0;
				}
			}
		}
	}

#endif
}


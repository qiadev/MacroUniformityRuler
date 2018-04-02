/*
 *  w11Matrix.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 4/30/08. 
 *
 */

#include "w11Matrix.h"

#include <iostream>
#include <string>
using namespace std;

#ifdef _W11_SUPPORT_TIFF_
#include "tiffio.h"


/// Write image to TIFF file. Pads the filepath by the "tif" extension.
/// - returns : Path to the written file.
std::string writeTiff(const w11Matrix<w11T8Bits>& matrix, string filepath, float xdpi, float ydpi, const string& description)
{
    filepath = filepath + ".tif";
	cout << "Writing TIFF image file" << endl;
	TIFF* tfile= TIFFOpen(filepath.c_str(), "w");
	if (!tfile) {
		throw string("Failed to open TIFF output file:") + filepath;
	}
	
	uint32 ncols= matrix.sizex();
	uint32 nrows= matrix.sizey();

	TIFFSetField(tfile, TIFFTAG_IMAGEWIDTH, ncols);
	TIFFSetField(tfile, TIFFTAG_IMAGELENGTH, nrows);
	TIFFSetField(tfile, TIFFTAG_FILLORDER, FILLORDER_LSB2MSB);
	TIFFSetField(tfile, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	
	TIFFSetField(tfile, TIFFTAG_RESOLUTIONUNIT, (uint16)RESUNIT_INCH);
	TIFFSetField(tfile, TIFFTAG_XRESOLUTION, xdpi);
	TIFFSetField(tfile, TIFFTAG_YRESOLUTION, ydpi);
	
    TIFFSetField(tfile, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tfile, TIFFTAG_SAMPLESPERPIXEL, 1);
    TIFFSetField(tfile, TIFFTAG_PHOTOMETRIC,PHOTOMETRIC_MINISBLACK);
	
	TIFFSetField(tfile, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tfile, TIFFTAG_ROWSPERSTRIP, 1L);
	
	TIFFSetField(tfile, TIFFTAG_IMAGEDESCRIPTION, description.c_str());
	
	long linesize= TIFFScanlineSize(tfile);
	tdata_t buf = _TIFFmalloc(linesize);
	if (!buf) {
		throw string("Failed to TIFFmalloc memory");
	}
	
	uint32 ix, iy;
	
	for (iy=0; iy<nrows; iy++) {
		uint8* pp = (uint8*) buf;
		for (ix=0; ix<ncols; ix++) {
			*pp++ = matrix[iy][ix];
		}
		if (TIFFWriteScanline(tfile, buf, iy, 0) < 0) {
			_TIFFfree(buf);
			throw string("writeTiff: Error writing scanline");
		}
	}
	TIFFClose(tfile);
	_TIFFfree(buf);

    return filepath;
}

#endif




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
// MARK: -  Private data members	
private:   
	w11Matrix<P> m;

// MARK: -  Public methods	
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

    std::string writeBMP(std::string filepath) const
    {
        filepath = filepath + ".bmp";
        writeBMPFile(filepath, false);
        return filepath;
    }

private:

    // MARK: - BMP support
    // Based on <https://stackoverflow.com/a/32027388/668253>.
    // See also <https://web.archive.org/web/20080912171714/http://www.fortunecity.com/skyscraper/windows/364/bmpffrmt.html>

    /// Write image to BMP file (image must be 8 bit). Pads the filepath by the bmp extension.
    /// - returns : Path to the written file.
    void writeBMPFile(std::string filepath, bool grayScale = false) const   // FIXME: grayscale not working!
    {
        unsigned int headers[13];
        FILE * outfile;
        int extrabytes;
        int paddedsize;
        int x; int y; int n;
        int red, green, blue;

        int samplesPerPixel;    // RGB:3 gray:1
        samplesPerPixel = grayScale ? 1 : 3;

        int WIDTH = (int) nx();
        int HEIGHT = (int) ny();

        extrabytes = (4 - ((WIDTH * samplesPerPixel) % 4)) % 4;  // DRR: One more %4, to avoid padding with 4.               // How many bytes of padding to add to each
        // horizontal line - the size of which must be a multiple of 4 bytes.

        paddedsize = ((WIDTH * samplesPerPixel) + extrabytes) * HEIGHT;

        // Headers...
        // Note that the "BM" identifier in bytes 0 and 1 is NOT included in these "headers".

        headers[0]  = paddedsize + 54;      // bfSize (whole file size)
        headers[1]  = 0;                    // bfReserved (both)
        headers[2]  = 54;                   // bfOffbits
        headers[3]  = 40;                   // biSize
        headers[4]  = WIDTH;  // biWidth
        headers[5]  = HEIGHT; // biHeight

        // Would have biPlanes and biBitCount in position 6, but they're shorts.
        // It's easier to write them out separately (see below) than pretend
        // they're a single int, especially with endian issues...

        int xPixelsPerMeter = (int) 1000.0 / dx();
        int yPixelsPerMeter = (int) 1000.0 / dy();
        headers[7]  = 0;                    // biCompression
        headers[8]  = paddedsize;           // biSizeImage
        headers[9]  = xPixelsPerMeter;      // biXPelsPerMeter
        headers[10] = yPixelsPerMeter;      // biYPelsPerMeter
        headers[11] = 0;                    // biClrUsed
        headers[12] = 0;                    // biClrImportant

        outfile = fopen(filepath.c_str(), "wb");

        //
        // Headers begin...
        // When printing ints and shorts, we write out 1 character at a time to avoid endian issues.
        //

        fprintf(outfile, "BM");

        for (n = 0; n <= 5; n++)
        {
            fprintf(outfile, "%c", headers[n] & 0x000000FF);
            fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
            fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
            fprintf(outfile, "%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
        }

        // These next 4 characters are for the biPlanes and biBitCount fields.

        fprintf(outfile, "%c", 1);
        fprintf(outfile, "%c", 0);
        fprintf(outfile, "%c", grayScale ? 8 : 24);
        fprintf(outfile, "%c", 0);

        for (n = 7; n <= 12; n++)
        {
            fprintf(outfile, "%c", headers[n] & 0x000000FF);
            fprintf(outfile, "%c", (headers[n] & 0x0000FF00) >> 8);
            fprintf(outfile, "%c", (headers[n] & 0x00FF0000) >> 16);
            fprintf(outfile, "%c", (headers[n] & (unsigned int) 0xFF000000) >> 24);
        }

        //
        // Headers done, now write the data...
        //

        for (y = HEIGHT - 1; y >= 0; y--)     // BMP image format is written from bottom to top...
        {
            for (x = 0; x <= WIDTH - 1; x++)
            {
                red = constRowPtr(y)[x];
                green = blue = red;

                // Also, it's written in (b,g,r) format...
                if (grayScale) {
                    fprintf(outfile, "%c", red);
                } else {
                    fprintf(outfile, "%c", blue);
                    fprintf(outfile, "%c", green);
                    fprintf(outfile, "%c", red);
                }
            }
            if (extrabytes)      // See above - BMP lines must be of lengths divisible by 4.
            {
                for (n = 1; n <= extrabytes; n++)
                {
                    fprintf(outfile, "%c", 0);
                }
            }
        }

        fclose(outfile);

    }

   
};


#endif


#include "w11Defects.h"
#include "w11fft.h"

#include <iostream>
#include <string>
#include <complex>
#include <math.h>
using namespace std;

static const bool vertical= true;
static const bool horizontal= false;
static const bool upright= true;
static const bool downright= false;


w11ImageChannelT<w11T8Bits>* quantize(const w11ImageChannelT<w11T16Bits>& image)
{
	w11ImageChannelT<w11T8Bits> *im8= new w11ImageChannelT<w11T8Bits>(image.ny(), image.nx());
	im8->setScale(image.dx(), image.dy());
	
	long ix,iy;
	w11T16Bits v16;
	const w11T16Bits *v16ptr;
	w11T8Bits v8;
	double cvtf= 255.0 / 65535;
	double v255= 0;
	for (iy=0;iy<image.ny();iy++) {
		v16ptr= image.constRowPtr(iy);
		for (ix=0;ix<image.nx();ix++) {
			v16= *v16ptr++;
			v255= v16 * cvtf + (double)rand() / RAND_MAX; 
			if (v255 > 255) {
				v8= 255;
			} else {
				v8= static_cast<w11T8Bits>(v255);
			}
			(*im8)(iy,ix)= v8;
		}
	}
	return im8;
}


w11ImageChannelT<w11T8Bits>* quantize(const w11ImageChannelT<float>& image,const double vmin,const double vmax,const double noiseLevel)
{
	w11ImageChannelT<w11T8Bits> *im8= new w11ImageChannelT<w11T8Bits>(image.ny(), image.nx());
	im8->setScale(image.dx(), image.dy());
	
	long ix,iy;
	const float *fptr;
	w11T8Bits v8;
	double cvtf= 255.0 / (vmax - vmin);
	double v255= 0;
	for (iy=0;iy<image.ny();iy++) {
		fptr= image.constRowPtr(iy);
		for (ix=0;ix<image.nx();ix++) {
			v255= (*fptr++ - vmin) * cvtf + noiseLevel * (double)rand() / RAND_MAX;
			if (v255 > 255) {
				v8= 255;
			} else {
				v8= static_cast<w11T8Bits>(v255);
			}
			(*im8)(iy,ix)= v8;
		}
	}
	return im8;
}


static const double twopi = 2 * 3.14159265358998;
static const double gImageLength = IMAGE_SIZE;	// ≈ 173.4 mm

// Park and Miller's minimal standard random number generator:
// ... from Numerical Recipes in C, 2nd edition, p. 278
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876
// Set or reset idum to any integer value, except the value MASK.
// Do not change idum between calls.
float ran0(long *idum)	// random number generator
{
	long k;
    float ans;
    
    *idum ^= MASK;
    k= (*idum)/IQ;
    *idum= IA*(*idum - k*IQ) - IR*k;
    if (*idum < 0) {
        *idum += IM;
    }
    ans= (float)(AM * (*idum));
    *idum ^= MASK;
    return ans;    
}

#pragma mark w11MacroMask
//=========================== w11MacroMask =========================================

w11MacroMask::w11MacroMask(void) 
{}

w11MacroMask::~w11MacroMask()
{}

// returns modulation [0,1] at point (x,y) millimeter from (left,bottom).
// Normalized to peak value = 1
double w11MacroMask::mask(double x,double y)
{
    return 1.0;
}

#pragma mark w11MaskDiagonalLinear
//=========================== w11MaskDiagonalLinear =========================================

w11MaskDiagonalLinear::w11MaskDiagonalLinear(bool upright,double length) :
fUpright(upright),
fLength(length)
{
    fDiagonalLength= 2 * fLength;
}

w11MaskDiagonalLinear::~w11MaskDiagonalLinear()
{}

// returns modulation [0,1] at point (x,y) millimeter from (left,bottom).
// Normalized to peak value = 1
double w11MaskDiagonalLinear::mask(double x,double y)
{
    double dist= x+y;
    return fUpright ? dist / fDiagonalLength : 1 - dist / fDiagonalLength;
}

#pragma mark w11MaskFermi
//=========================== w11MaskFermi =========================================

w11MaskFermi::w11MaskFermi(bool xon,bool xdrop,double xc,double xw,
                           bool yon,bool ydrop,double yc,double yw) :
fXActive(xon), fXDrop(xdrop), fX(xc), fXWidth(xw),
fYActive(yon), fYDrop(ydrop), fY(yc), fYWidth(yw)
{
    
}

w11MaskFermi::~w11MaskFermi()
{}

// returns modulation [0,1] at point (x,y) millimeter from (left,bottom).
// Normalized to peak value = 1
double w11MaskFermi::mask(double x,double y)
{
	double m;
    if (fXActive) {
        double dxs= fXDrop ? (x - fX)/fXWidth : (fX - x)/fXWidth;
        m= 1.0 / (1 + exp(dxs));
    } else {
        m= 1;
    }
    if (fYActive) {
        double dys= fYDrop ? (y - fY)/fYWidth : (fY - y)/fYWidth;
        m *= 1.0 / (1 + exp(dys));
    }
    return m;
}

#pragma mark w11MaskFermiRadial
//=========================== w11MaskFermiRadial =========================================

w11MaskFermiRadial::w11MaskFermiRadial(bool invert,
                                       bool xon,double xc,double xw,
                                       bool yon,double yc,double yw) :
fInvert(invert),
fXActive(xon), fX(xc), fXWidth(xw),
fYActive(yon), fY(yc), fYWidth(yw)
{
    
}

w11MaskFermiRadial::~w11MaskFermiRadial()
{}


// returns modulation [0,1] at point (x,y) millimeter from (left,bottom).
// Normalized to peak value = 1
double w11MaskFermiRadial::mask(double x,double y)
{
	double m;
    if (fXActive) {
        double rx= fabs(x - fX);    // Form plateau with radius fXWidth
        double dxs= rx/fXWidth;
        m= 2.0 / (1 + exp(dxs));
    } else {
        m= 1;
    }
    if (fYActive) {
        double ry= fabs(y - fY);    // Form plateau with radius fYWidth
        double dys= ry/fYWidth;
        m *= 2.0 / (1 + exp(dys));
    }
    if (fInvert) {
        m= 1 - m;   // doesn't work !!
    }
    //cout << x << " " << y << " " << m << endl;
    return m;
}

#pragma mark w11MacroDefect
//=========================== w11MacroDefect =========================================

w11MacroDefect::w11MacroDefect(double amplitude,unsigned long seed,double dxy) :
fAmplitude(amplitude), fDeltaXY(dxy),fRandomSeed(seed)
{
    fMask= NULL;
}

w11MacroDefect::~w11MacroDefect()
{
    if (fMask) {
        delete fMask;
    }
}

void w11MacroDefect::assignMask(w11MacroMask *mask)
{
    fMask= mask;
}

double w11MacroDefect::csf(const double cpmm)
{
    // based on viewing distance 400 mm
    const double a= 3.5499;
    const double b= -1.39626;
    const double c= 0.8;
    
    double m= a * pow(cpmm,c) * exp(b * cpmm);
    // don't go lower than 0.5 at low frequencies
    //if (cpmm < 0.5 && m < 0.5) {
    //    m= 0.5;
    //}
    return m;
}

float w11MacroDefect::random(void)
{
    fRngCount++;
    return ran0(&fRandomSeed);
}


#pragma mark w11Banding

//=========================== w11Banding =========================================

w11Banding::w11Banding(double amp,unsigned long seed,double dxy,double lambda,
                                 double degPhase,bool vertical) :
w11MacroDefect(amp,seed,dxy),
fLambda(lambda),
fPhase(degPhase/360*twopi),
fVertical(vertical)
{
}

w11Banding::~w11Banding()
{}

double w11Banding::dlevel(double x,double y)
{
    double distance= fVertical ? x : y;
    double level= fAmplitude * cos(fPhase + twopi*distance/fLambda);
    if (fMask) {
        level *= fMask->mask(x,y);
    }
    return level;    
}

#pragma mark w11BandingSweep

//=========================== w11BandingSweep =========================================

w11BandingSweep::w11BandingSweep(double amp,unsigned long seed,double dxy,double lambda1,double lambda2,
                                 double degPhase,double length,bool vertical) :
w11MacroDefect(amp,seed,dxy),
fLambda1(lambda1),
fLambda2(lambda2),
fPhase(degPhase/360*twopi),
fLength(length),
fVertical(vertical)
{
    fDeltaLambda= fLambda2 - fLambda1;
    fDeltaLambdaOverL= fDeltaLambda / fLength;
    fTwoPiLOverDeltaLambda= twopi * fLength / fDeltaLambda;
}

w11BandingSweep::~w11BandingSweep()
{}

double w11BandingSweep::dlevel(double x,double y)
{
    //double distance= fDeltaXY * (fVertical ? x : y);
    double distance= fVertical ? x : y;
    double p1= fLambda1 + distance * fDeltaLambdaOverL;
    double f= 1 / p1;
    double Mf= 1 / csf(f);
    //double Mf= 1 ;
    double level= fAmplitude * Mf * cos(fPhase + fTwoPiLOverDeltaLambda * log(p1));
    //cout << distance << ", " << level << endl;
    if (fMask) {
        level *= fMask->mask(x,y);
    }
    return level;    
}

#pragma mark w11RandomStreaks
//=========================== w11RandomStreaks =========================================

w11RandomStreaks::w11RandomStreaks(double amp,unsigned long seed,double dxy,double pmin,double pmax,bool vertical) :
w11MacroDefect(amp,seed,dxy),
fPeriodMin(pmin),
fPeriodMax(pmax),
fVertical(vertical)
{
    fSignalLength= gImageLength;	//mm
	fDelta= fSignalLength / NRS;
	w11FFT<float> fft1d(NRS);
	w11complex<float> cdata[NRS];
    float *profile= new float[NRS];
    
    double fmin= 1.0 / fPeriodMax;
    double fmax= 1.0 / fPeriodMin;
    double f= 0;
    double theta= 0;
    double df= 1.0/fSignalLength;
    double Mf= 0;
	double mean= 0;
    double rnd=0; 
	long i= 0;
    //cout << "df:" << df << " fmin:" << fmin << " fmax:" << fmax << endl;
	cdata[0].real()= (float) mean;
	cdata[0].imag()= 0;			// no impact on real component
    for (i= 1; i <= NRS/2; i++) {
        rnd= random();		// always get the random number;
        f= i*df;
        Mf= 1 / csf(f);
        if (f >= fmin && f <= fmax) {
			theta= twopi * rnd;
			cdata[i].real()= (float)(Mf * cos(theta)); 
			cdata[i].imag()= (float)(Mf * sin(theta)); 
        } else {
			cdata[i].real()= 0; 
			cdata[i].imag()= 0; 
        }
    }
	// set remaining components to 0:
	for (; i<NRS; i++) {
		cdata[i].real()= 0;
		cdata[i].imag()= 0;
	}
    
    // FFT to real space:
	fft1d.transformComplexData(cdata, false);
	fft1d.getRealData(profile);
    for (long i=0;i<NRS;i++) {
        fSignal[i]= profile[i];
    }
    
    // normalize:
    double vmax= fSignal[0];
    double vmin= fSignal[0];
    for (long i=0;i<NRS;i++) {
        if (fSignal[i] < vmin) {
            vmin= fSignal[i];
        } else if (fSignal[i] > vmax) {
            vmax= fSignal[i];
        }
    }
    double vrange= vmax - vmin;
    double fnorm;
    if (vrange > 0) {
	    fnorm= fAmplitude * 2 / vrange;
    } else {
        fnorm= 1;
    }
    for (long i=0;i<NRS;i++) {
        fSignal[i] *= fnorm;
        //cout << i*fDeltaXY << " " <<fSignal[i] << endl;
    }
    
    delete [] profile;
}

w11RandomStreaks::~w11RandomStreaks()
{}

double w11RandomStreaks::dlevel(double x,double y)
{
	double d= fVertical ? y : x;				// 2009-05-02: Error here, should switch x and y, but leaving it as is to not change standard image!
    long index= static_cast<long>(d / fDelta);
    // cout << index << " " << NRS << endl;
    if (index < 0) index= 0;
    if (index >= NRS) index= NRS-1;
    
    double level= fSignal[index];
    //cout << level << endl;
    
    if (fMask) {
        level *= fMask->mask(x,y);
        //cout << level << endl;
    }
    return level;    
}


#pragma mark w11IsolatedStreaks
//=========================== w11IsolatedStreaks =========================================


w11IsolatedStreaks::w11IsolatedStreaks(double amp,unsigned long seed,double dxy,double location,double width,bool bright,bool vertical) :
w11MacroDefect(amp,seed,dxy),
fCenterLocation(location),
fWidth(width),
fVertical(vertical),
fBright(bright)
{
}

w11IsolatedStreaks::~w11IsolatedStreaks()
{}

double w11IsolatedStreaks::dlevel(double x,double y)
{
    double pos= fVertical ? x : y;
    double level=0;
	
	// if pos is more than fWidth/2 away from center location, then there is no effect:
	double dpos= pos - fCenterLocation;
	if (fabs(dpos) > fWidth/2.0) {
		return 0;
	}
	// calculate pos in units of full cycle [-pi,pi]:
	double rpos= dpos * twopi / fWidth;
	level= fAmplitude * (1 + cos(rpos))/2.0;
	if (!fBright) {
		level= -level;
	}
	return level;
}


#pragma mark w11Mottle
//=========================== w11Mottle =========================================
w11Mottle::w11Mottle(double amp,unsigned long seed,double dxy,double pmin,double pmax) :
w11MacroDefect(amp,seed,dxy),
fPeriodMin(pmin),
fPeriodMax(pmax),
nsize(NRS)		// NRS = 2048 default
{
    //cout << "gImageLength=" << gImageLength << endl;
    fDelta= gImageLength / nsize;  // mm spatial delta
	//.. w11ImageChannelT<complex<float> > fftimage(nsize, nsize);
	w11Matrix< w11complex<float> > cmatrix(nsize, nsize);
    double df= 1.0/(nsize*fDelta);			// frequency step
	//.. fftimage.setScale(df, df);
    
    double fmin= 1.0 / fPeriodMax;
    double fmax= 1.0 / fPeriodMin;
    double theta= 0,fx,fy,absfreq;
    double rnd;
    double Mf;
    
    //cout << "f range: " << fmin << " " << fmax << endl;
    
    long nx= cmatrix.sizex();
	long ny= cmatrix.sizey();
	// set cdata to zero:
	for (long iy=0;iy<ny;iy++) {
		for (long ix=0;ix<nx;ix++) {
			cmatrix[iy][ix]= 0;
		}
	}
	// keep mean and 1D frequencies to zero (stay away from iy=0 and ix=0)
	for (long iy=1;iy<ny;iy++) {
		if (iy <= ny/2) {
			fy= iy * df;
		} else {
			fy= (ny-iy) * df;
		}
		for (long ix=1;ix<nx;ix++) {
            rnd= random();			// always call it
			if (ix <= nx/2) {
				fx= ix * df;
			} else {
				fx= (nx-ix) * df;
			}
			absfreq= sqrt(fx*fx + fy*fy);
            if ((absfreq >= fmin) && (absfreq <= fmax)) {	
                Mf= 1 / csf(absfreq);
                //cout << "f=" << absfreq << " rnd=" << rnd << " Mf=" << Mf << endl;
                theta= twopi * rnd;
				cmatrix[iy][ix].real()= (float)(Mf * cos(theta));
				cmatrix[iy][ix].imag()= (float)(Mf * sin(theta));
            }
		}
	}
	w11FFT2D<float> fft2d(ny, nx);
	fft2d.setComplexMatrix(cmatrix);
	fft2d.transform(false);
	w11Matrix<float> rmatrix(ny, nx);
	fft2d.getRealComponent(rmatrix);
	image= new w11ImageChannelT<float>(ny,nx);
	for (long iy=0;iy<nsize;iy++) {
		for (long ix=0;ix<nsize;ix++) {
			(*image)(iy, ix)= rmatrix[iy][ix];
		}
	}
	
	float vmin=0, vmax=0;
	image->getRange(vmin, vmax);
    double vrange= vmax - vmin;
    if (vrange > 0) {
		image->matrix() *= (float)((fAmplitude * 2.0/vrange));
    }
}

w11Mottle::~w11Mottle()
{
    delete image;
}


double w11Mottle::dlevel(double x,double y)
{   
    long ix= static_cast<long>(x / fDelta) % nsize;
    long iy= static_cast<long>(y / fDelta) % nsize;
    
    double level= (*image)(ix,iy);
    if (fMask) {
        level *= fMask->mask(x,y);
    }
    return level;    
}

#pragma mark w11MacroScale
//=========================== w11MacroScale =========================================

w11MacroScale::w11MacroScale(void)
{
    init();
}


w11MacroScale::~w11MacroScale()
{
}

void w11MacroScale::addDefect(w11MacroDefect* d)
{
    fDefects.push_back(d);
}

// generate image with defects;
// size given in mm by width x height;
// average graylevel given by area coverage ac in range 0 to 100;
w11ImageChannelT<float>* w11MacroScale::generateFloat(double width,double height,double ac,double dpi)
{    
	w11ImageChannelT<float> *image= NULL;
	float *fptr= NULL;
	
	// calc raster size:
	w11Tulong nx,ny;
	nx= static_cast<w11Tulong>(width / 25.4 * dpi);
	ny= static_cast<w11Tulong>(height / 25.4 * dpi);
	// allocate:
	image= new w11ImageChannelT<float>(ny, nx);
	if (!image) throw string("w11MacroScale::generate image allocationfailed");
	// resolution:
	double dx= 25.4 / dpi;
	double dy= dx;
	image->setScale(dx,dy);
	
	w11Tulong ix,iy;
	double x,y;     // location
	list<w11MacroDefect*>::iterator dit;
	double accumulatedLevel= 0;    // sum of defect levels
	float level, delta=0;
	for (iy=0;iy<ny;iy++) {
		fptr= image->rowPtr(iy);
		y= iy * dy;
		for (ix=0;ix<nx;ix++) {
			x= ix * dx;
			accumulatedLevel= 0;
			for (dit= fDefects.begin(); dit != fDefects.end(); ++dit) {
				delta= (float)((*dit)->dlevel(x,y));
				//cout << delta << endl;
				accumulatedLevel += delta;
			}
			level= (float)(ac + accumulatedLevel);
			*fptr++ = level;
		}
	}
	return image;
}


// generate image with defects;
// size given in mm by width x height;
// average graylevel given by area coverage ac in range 0 to 100;
w11ImageChannelT<w11T16Bits>* w11MacroScale::generate(double width,double height,double ac,double dpi)
{    
    w11ImageChannelT<w11T16Bits> *image= NULL;
    w11T16Bits *pixptr= NULL;
    
    // calc raster size:
    w11Tulong nx,ny;
    nx= static_cast<w11Tulong>(width / 25.4 * dpi);
    ny= static_cast<w11Tulong>(height / 25.4 * dpi);
    // allocate:
    image= new w11ImageChannelT<w11T16Bits>(ny, nx);
    if (!image) throw string("w11MacroScale::generate image allocationfailed");
    // resolution:
    double dx= 25.4 / dpi;
    double dy= dx;
    image->setScale(dx,dy);
    
    w11Tulong ix,iy;
    double x,y;     // location
    list<w11MacroDefect*>::iterator dit;
    double accumulatedLevel= 0;    // sum of defect levels
    double level, delta=0;
    for (iy=0;iy<ny;iy++) {
		pixptr= image->rowPtr(iy);
        y= iy * dy;
        for (ix=0;ix<nx;ix++) {
            x= ix * dx;
            accumulatedLevel= 0;
            for (dit= fDefects.begin(); dit != fDefects.end(); ++dit) {
                delta= (*dit)->dlevel(x,y);
                //cout << delta << endl;
                accumulatedLevel += delta;
            }
            level= ac + accumulatedLevel;
            long ilevel= static_cast<long>(655.35 * level);
            if (ilevel < 0 || ilevel > 65535) {
				cout << "Level out of range:" << level << " " << ilevel << endl;
				throw string("w11MacroScale::generate - level out of range.");
            }
            *pixptr++ = static_cast<w11T16Bits>(ilevel);
        }
    }
    
    return image;
}


void w11MacroScale::init(void)
{
    string		mClientIdent= "ISO";
    string      fHomeDirectory("/tmp/w11macro/");
    
    //bool ok= createContainingDirectory(fHomeDirectory, false);
	
}

w11ImageChannelT<float>* w11MacroScale::makeDefectImage(double meanY, double amplitude)
{
	w11MacroScale   fScale;
	
	double contrast= amplitude;       // overall contrast
	double dpi= DPI;
	double imageWidth= IMAGE_SIZE;          // mm
	double imageHeight= IMAGE_SIZE;         // mm
	double meanAC= meanY;
	double transitionWidth= 20;	// 20 is good
	w11MaskFermi   *mask=0;
	w11BandingSweep *bs=0;
	w11RandomStreaks *rs=0;
	w11Mottle *mottle=0;
	double dxy= 25.4 / dpi; // the pixel size;
	//w11IsolatedStreaks *ws;    
	
	//rs= new w11RandomStreaks(2.0, 1, dxy, 2, 20, vertical);
	//return;
	
#if 0 // this is for the EI 2006 paper
	rs= new w11RandomStreaks(contrast * 1.2, 6002, dxy, 5, 50, horizontal);
	fScale.addDefect(rs);
#endif
	
	
#if 1 // within this #if is the code used for first ruler, 2005.10 survey
	
#if 1
	// mottle :======================= #1, 2, 3
	cout << "\t\t\tMottle\t\t#1001..." << endl;
	mottle= new w11Mottle(contrast * 1, 1001, dxy, 5, 15);
	fScale.addDefect(mottle);
	cout << "\t\t\tMottle\t\t#1002..." << endl;
	mottle= new w11Mottle(contrast * 1, 1002, dxy, 15, 45);
	fScale.addDefect(mottle);
	cout << "\t\t\tMottle\t\t#1003..." << endl;
	mottle= new w11Mottle(contrast * 1, 1003, dxy, 45, 150);
	fScale.addDefect(mottle);
#endif
	
#if 1
	// Center region #4
	w11MaskFermiRadial *rmask= new w11MaskFermiRadial(false, true, imageWidth/2, transitionWidth, true,imageHeight/2, transitionWidth);
	// high-freq banding:
	cout << "\t\t\tBandingSweep\t\t#2001" << endl;
	bs= new w11BandingSweep(contrast * 1.2, 2001, dxy, 0.5, 2, 0, imageWidth, vertical);
	bs->assignMask(rmask);
	fScale.addDefect(bs);
#endif
	
	// vertical bands Q1 :=======================
	mask= new w11MaskFermi(true, true, imageWidth/2, transitionWidth, true, true, imageHeight/2, transitionWidth);
#if 1	// #5
	//bs= new w11BandingSweep(contrast * .7, 3001, dxy, 3, 25, 0, imageWidth, vertical);
	cout << "\t\t\tBandingSweep\t\t#3001" << endl;
	bs= new w11BandingSweep(contrast * .4, 3001, dxy, 3, 25, 0, imageWidth, vertical);
	bs->assignMask(mask);
	fScale.addDefect(bs);
#endif
#if 1	// #6
	// secondary:
	cout << "\t\t\tRandomStreaks\t\t#3002" << endl;
	rs= new w11RandomStreaks(contrast * 1.2, 3002, dxy, 2, 10, vertical);
	rs->assignMask(mask);
	fScale.addDefect(rs);
#endif
	
	// horizontal bands Q2:
	mask= new w11MaskFermi(true, false, imageWidth/2, transitionWidth, true, true, imageHeight/2, transitionWidth);
	//
#if 1	// #7
	//bs= new w11BandingSweep(contrast * .7, 5001, dxy, 3, 25, 0, imageWidth, horizontal);
	cout << "\t\t\tBandingSweep\t\t#5001" << endl;
	bs= new w11BandingSweep(contrast * .4, 5001, dxy, 3, 25, 0, imageWidth, horizontal);
	bs->assignMask(mask);
	fScale.addDefect(bs);
#endif
#if 1	// #8
	// secondary:
	cout << "\t\t\tRandomStreaks\t\t#5002" << endl;
	rs= new w11RandomStreaks(contrast * 1.2, 5002, dxy, 2, 50, horizontal);
	rs->assignMask(mask);
	fScale.addDefect(rs);
#endif
#if 1	// #9
	// high-freq banding:
	cout << "\t\t\tBandingSweep\t\t#5003" << endl;
	bs= new w11BandingSweep(contrast * 1, 5003, dxy, 0.5, 3.0, 0, imageWidth, horizontal);
	bs->assignMask(mask);
	fScale.addDefect(bs);
#endif
	
	// horizontal bands Q3:
	mask= new w11MaskFermi(true, true, imageWidth/2, transitionWidth, true, false, imageHeight/2, transitionWidth);
	//
#if 1	// #10
	cout << "\t\t\tBandingSweep\t\t#6001" << endl;
	bs= new w11BandingSweep(contrast * 0.4, 6001, dxy, 5, 25, 0, imageHeight, horizontal);
	bs->assignMask(mask);
	fScale.addDefect(bs);
#endif
#if 1	// #11
	// secondary:
	cout << "\t\t\tRandomStreaks\t\t#6002" << endl;
	rs= new w11RandomStreaks(contrast * 1.2, 6002, dxy, 5, 50, horizontal);
	rs->assignMask(mask);
	fScale.addDefect(rs);
#endif
#if 1	// #12
	// high-freq banding:
	cout << "\t\t\tBandingSweep\t\t#6003" << endl;
	bs= new w11BandingSweep(contrast * 1, 6003, dxy, 0.5, 2.0, 0, imageHeight, horizontal);
	bs->assignMask(mask);
	fScale.addDefect(bs);
	
#endif
	
	//- vertical bands Q4 (bottom, right)
	mask= new w11MaskFermi(true, false, imageWidth/2, transitionWidth, true, false, imageHeight/2, transitionWidth);
	//
#if 1	// #13
	cout << "\t\t\tBandingSweep\t\t#4001" << endl;
	bs= new w11BandingSweep(contrast * 0.4, 4001, dxy, 10, 25, 0, imageWidth, vertical);
	bs->assignMask(mask);
	fScale.addDefect(bs);
#endif
#if 1	// #14
	// secondary:
	cout << "\t\t\tRandomStreaks\t\t#4002" << endl;
	rs= new w11RandomStreaks(contrast * 1.2, 4002, dxy, 2, 20, vertical);
	rs->assignMask(mask);
	fScale.addDefect(rs);
#endif
	/*
	 #if 0
	 // high-freq banding:
	 bs= new w11BandingSweep(contrast * 1, 4003, dxy, 0.1, 0.4, 90, imageWidth, vertical);
	 bs->assignMask(mask);
	 fScale.addDefect(bs);
	 #endif
	 */
	
#endif
	
	cout << "Defects completed" << endl;
	cout << "Rendering defects..." << endl;
	w11ImageChannelT<float> *image= fScale.generateFloat(imageWidth,imageHeight,meanAC,dpi);
	return image;
}


#pragma mark Global functions

w11T16Bits cvtfloat(float pixelvalue)
{
	return static_cast<w11T16Bits>(655.35 * pixelvalue);
}

w11ImageChannelT<float>& trcCorrect(w11ImageChannelT<float>& image, w11CalibrationProcess& cp)
{
	float *fptr= 0;
	for (long iy=0;iy<image.ny();iy++) {
		fptr= image.rowPtr(iy);
		for (long ix=0;ix<image.nx();ix++) {
			*fptr= (float) cp.calibrate(*fptr);
			fptr++;
		}
	}
	return image;
}








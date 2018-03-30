#ifndef __w11_defects_h__
#define __w11_defects_h__

/** @file
 *
 * Defect levels are on a scale from 0 - 100 (L* like)
 */


#include <list>
#include <sstream>
using namespace std;

#include "w11Types.h"
#include "w11ImageChannelT.h"
#include "w11CalibrationProcess.h"

#define NRS	2048                                // should be 2048
#define IMAGE_SIZE  173                         // shold be calculated = NRS * 25.4 / DPI
#define DPI	(NRS*25.4/IMAGE_SIZE)               // should be 300


//=========================== w11MacroMask =========================================
class w11MacroMask {
public:
    w11MacroMask(void);
    virtual ~w11MacroMask();
    
    virtual double mask(double x,double y);     // returns modulation [0,1] at point (x,y) millimeter from (left,bottom).
	// Normalized to peak value = 1
    
private:
};

//=========================== w11MaskDiagonalLinear =========================================
class w11MaskDiagonalLinear : public w11MacroMask {
public:
    w11MaskDiagonalLinear(bool upright,double length);
    ~w11MaskDiagonalLinear();
    
    double mask(double x,double y);     
    
private:
	bool fUpright;
    double fLength;
    double fDiagonalLength;
};

//=========================== w11MaskFermi =========================================
class w11MaskFermi : public w11MacroMask {
public:
    w11MaskFermi(bool xon,bool xdrop,double xc,double xw,
                 bool yon,bool ydrop,double yc,double yw);
    ~w11MaskFermi();
    
    double mask(double x,double y);     
    
private:
	bool 	fXActive;		// x mask active
    bool 		fXDrop;			// x mask drops at higher values
    double		fX;				// x cutoff
    double		fXWidth;		// transition width
    bool 		fYActive;		// y mask active
    bool 		fYDrop;			// y mask drops at higher values
    double		fY;				// y cutoff
    double		fYWidth;		// transition width
};

/** @class w11MaskFermiRadial
 *
 */
class w11MaskFermiRadial : public w11MacroMask {
public:
    w11MaskFermiRadial(bool invert,
                       bool xon,double xc,double xw,
                       bool yon,double yc,double yw);
    ~w11MaskFermiRadial();
    
    double mask(double x,double y);     
    
private:
	bool    fInvert;        /**< mask the center if true */
    bool 	fXActive;		// x mask active
    double		fX;				// x center
    double		fXWidth;		// transition width
    bool 		fYActive;		// y mask active
    double		fY;				// y center
    double		fYWidth;		// transition width
};

//=========================== w11MacroDefect =========================================
class w11MacroDefect {
public:
    /** \brief Constructor, specifying amplitude, RNG seed, and spatial resolution [mm] 
	 *
	 * @param amplitude  The amplitude using scale from 0 to 100.
	 * @param seed       The seed for the random number generator for this object.
	 * @param dxy        The pixel size in millimeter; must be identical in the two directions.
	 */
    w11MacroDefect(double amplitude,unsigned long seed,double dxy);
    
    virtual ~w11MacroDefect();
    
    virtual double      dlevel(double x,double y)=0;       // returns defect level at location (x,y) millimeters
    void        assignMask(w11MacroMask *mask);
	
	std::string string(void) {
		std::ostringstream ost;
		return "Undefined";
	};
	
    
    
protected:
	const double      fAmplitude;
    const double            fDeltaXY;      
    w11MacroMask            *fMask;
    
    /** \brief Returns random number in [0,1]
	 *
	 */
    float random(void);
    
    /** \brief Returns number of times random() has been called on this object.
	 *
	 */
    long randomCount(void) {return fRngCount;};
    
    /**
	 * \brief Returns the human visual contrast sensitivity at given frequency [c/mm]
     *
     *	This assumes a certain view distance (400 mm).
     *
     * @param cpmm	The spatial frequency in cycles per mm on the print.
     *
     */
    double csf(const double cpmm);
    
private:
	long fRandomSeed;
    long fRngCount;			/**< Counts how many times the random number generator was called. */
};

/** @class w11Banding
 * \brief Banding with constant period lambda.
 *
 */
class w11Banding : public w11MacroDefect {
public:
    w11Banding(double amp,unsigned long seed,double dxy,double lambda,double degPhase,bool vertical);
    virtual ~w11Banding();
    
    virtual double dlevel(double x,double y);
	
	std::string string(void) {
		std::ostringstream ost;
		ost << "Banding: A=" << fAmplitude << " Lambda=" << fLambda << " Phase=" << fPhase << " Direction=" << (fVertical? "Vertical":"Horizontal");
		return ost.str();
	};
	
    
private:
	// independent parameters:
	const double fLambda;
    const double fPhase;
    const bool fVertical;
};

/** @class w11BandingSweep 
 * \brief Banding with period increasing linearly from lambda1 to lambda2, modulated by inverse CSF
 *
 */
class w11BandingSweep : public w11MacroDefect {
public:
    w11BandingSweep(double amp,unsigned long seed,double dxy,double lambda1,double lambda2,double degPhase,double length,bool vertical);
    virtual ~w11BandingSweep();
    
    virtual double dlevel(double x,double y);
	
	std::string string(void) {
		std::ostringstream ost;
		ost << "BandingSweep: A=" << fAmplitude << " Lambda1=" << fLambda1 << " Lambda2=" << fLambda2 << " Phase=" << fPhase << " Direction=" << (fVertical? "Vertical":"Horizontal");
		return ost.str();
	};
	
    
private:
	// independent parameters:
	const double fLambda1;
    const double fLambda2;
    const double fPhase;
    const double fLength;
    const bool fVertical;
    // derived parameters:
    double fDeltaLambda;
    double fDeltaLambdaOverL;
    double fTwoPiLOverDeltaLambda;
};

/** @class w11RandomStreaks
 * \brief Random streaks either horizontally or vertically.
 *
 */
class w11RandomStreaks : public w11MacroDefect {
public:
    w11RandomStreaks(double amp,unsigned long seed,double dxy,double pmin,double pmax,bool vertical);
    virtual ~w11RandomStreaks();
    
    virtual double dlevel(double x,double y);

	std::string string(void) {
		std::ostringstream ost;
		ost << (fVertical ? "Vertical" : "Horizontal");
		ost << " Random Streaks: A=" << fAmplitude << " Periods from " << fPeriodMin << " to " << fPeriodMax << " mm";
		return ost.str();
	};
    
private:
	/** independent parameters: */
	const double fPeriodMin;
    const double fPeriodMax;
    const bool fVertical;
    /** derived parameters: */
    double fSignalLength;		/**< Length [mm] of the space domain signal */
    double fSignal[NRS];		/**< The 1-dimensional profile */
    double fDelta;				/**< Stepsize [mm] of the signal (fSignal) */
    
    static double fRandom;		/**< The seed used for RNG */
};

//=========================== w11IsolatedStreaks =========================================
// narrow isolated streaks
class w11IsolatedStreaks : public w11MacroDefect {
public:
    w11IsolatedStreaks(double amp,unsigned long seed,double dxy,double location, double width,bool bright,bool vertical);
    virtual ~w11IsolatedStreaks();
    
    virtual double dlevel(double x,double y);

	std::string string(void) {
		std::ostringstream ost;
		ost << (fVertical ? "Vertical" : "Horizontal");
		ost << " Isolated Streak: A=" << fAmplitude << " Location: " << fCenterLocation << " Width: " << fWidth << " mm";
		return ost.str();
	};
	
	
private:
	const double fWidth;
	const double fCenterLocation;
    const bool fVertical;
    const bool fBright;
	//    
};

/** @class w11Mottle
 * \brief 2-dimensional noise
 */
class w11Mottle : public w11MacroDefect {
public:
    w11Mottle(double amp,unsigned long seed,double dxy,double pmin,double pmax);
    virtual ~w11Mottle();
    
    virtual double dlevel(double x,double y);
	
	std::string string(void) {
		std::ostringstream ost;
		ost << "Mottle: A=" << fAmplitude << " Periods from " << fPeriodMin << " to " << fPeriodMax << " mm";
		return ost.str();
	};
    
private:
	/** independent parameters: */
	const double fPeriodMin;
    const double fPeriodMax;
    const long nsize;
    /** derived parameters: */
    w11ImageChannelT<float> *image;
    double      fDelta;
};

//=========================== w11MacroScale =========================================
class w11MacroScale {
public:
    w11MacroScale(void);
    virtual ~w11MacroScale();
    
    void        init(void);
    
    void        addDefect(w11MacroDefect* d);
    
    w11ImageChannelT<float>* makeDefectImage(double meanY, double amplitude);
    
    virtual w11ImageChannelT<float>* generateFloat(double width,double height,double ac,double dpi);
    virtual w11ImageChannelT<w11T16Bits>* generate(double width,double height,double ac,double dpi);        // generates the raster data;
    
private:
	double      fAmplitude;     // overall amplitude (affects all defects)
    
    // defects:
    list<w11MacroDefect*>    fDefects;
};

w11ImageChannelT<float>& trcCorrect(w11ImageChannelT<float>& image, w11CalibrationProcess& cp);	/**< in-place correction; returns reference to same image */
w11ImageChannelT<w11T8Bits>* quantize(const w11ImageChannelT<w11T16Bits>& image);
w11ImageChannelT<w11T8Bits>* quantize(const w11ImageChannelT<float>& image,const double vmin,const double vmax,const double noiseLevel=1);

//float inverseTrc(float target);

w11T16Bits cvtfloat(float pixelvalue);

#endif


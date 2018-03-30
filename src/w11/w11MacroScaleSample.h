#ifndef _iso_macro_scale_sample_h
#define _iso_macro_scale_sample_h

/** @file
 *
 */

#include <string>
#include <fstream>
using namespace std;

#include "w11Types.h"
#include "w11ImageChannelT.h"
#include "w11CalibrationProcess.h"

class w11MacroScaleSample {
private:
	// Characteristics of the defect image:
	float fYMinimum;
	float fYMaximum;
	double fYMean;
	
	double fElMargin;
	
	double fYRms;

	// Defect region:
	double fDefectLeft;
	double fDefectTop;
	double fDefectWidth;
	double fDefectHeight;
	
	// TRC:
	double fTrcPatchWidth;
	double fTrcPatchHeight;
	double fTrcMargin;
	
	w11ImageChannelT<float> *fPage;
	
public:
	w11MacroScaleSample(const double width,const double height,const int dpi);
	~w11MacroScaleSample();
	
	void compose(double ymean, double amplitude, string id, bool includeDefect=true);
	// full region simulated defects:
	void simulatedVerticalBands(double a, double fcpmm);
	void simulatedHorizontalBands(double a, double fcpmm);

	//
	const w11ImageChannelT<float>& image(void);
	
	void fillPage(void);		// fill with current fYMean
	void fillPage(float val);
	void outlineRegion(double left,double top,double width,double height,double rimvalue);
	void setFlatField(double left,double top,double width,double height,double value);
	void copyToDefectRegion(const w11ImageChannelT<float> &image);
	void addDefectImage(long *pwidth,long *pheight,double meanY,double amplitude);
	void addTrc(double left,double top,double width,double height,double ystart,double yend,long nlevels);
	void addVerticalBands(double left,double top,double width,double height,double a,double fcpmm,bool replace=true);
	void addHorizontalBands(double left,double top,double width,double height,double a,double fcpmm, bool replace= true);
	void addUniformGray(double left,double top,double width,double height,double yvalue);
	
	void addFiducials(void);
	
	w11ImageChannelT<w11T16Bits>* paint(void);   /**< XYZ space, range [0,100] */
	void calibrate(w11CalibrationProcess& cp);
	
private:
	ofstream logfile;
	void makeMark(double x,double y);
	double rmsOffsetLevel(double frms);
	
};


#endif


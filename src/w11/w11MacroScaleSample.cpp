#include "w11MacroScaleSample.h"
#include "w11defects.h"
#include "w11Utilities.h"

#include <iostream>
#include <string>
#include <math.h>



w11MacroScaleSample::w11MacroScaleSample(const double width,const double height,const int dpi)
{
	double dx= 25.4 / dpi;
	double dy= dx;
	long pwidth= static_cast<long>(width / dx);
	long pheight= static_cast<long>(height / dy);

	fDefectLeft= 18;
	fDefectTop= 45;
	fDefectWidth= 170;
	fDefectHeight= 170;
	fYMean= 20.0;

	fYMinimum= fYMaximum= -999;
	fYRms= -999;
		
	fPage= new w11ImageChannelT<float>(pheight, pwidth);
	fPage->setScale(dx,dy);
	
	fElMargin= 0.20;
}

w11MacroScaleSample::~w11MacroScaleSample()
{
	delete fPage;
}

const w11ImageChannelT<float>& w11MacroScaleSample::image(void)
{
	return *fPage;
}

void w11MacroScaleSample::calibrate(w11CalibrationProcess& cp)
{
	trcCorrect(*fPage, cp);
}


void w11MacroScaleSample::compose(double ymean, double amplitude, string id, bool includeDefect)
{
	double vnext= 0, lnext=0;
	double defectLeft= fDefectLeft;									// 30;
	double defectTop= fDefectTop;									// 50;
	double bandLength= defectLeft - fElMargin - 0.1;		// 20;
	double defectAmplitude= amplitude;

	fYMean= ymean;
	
	string logfilename= string("log.") + id + ".txt";
	string logfilepath= w11Utility::appendPathSegment(w11Utility::instance().homeDirectory(), string("logs"));
	logfilepath= w11Utility::appendPathSegment(logfilepath, logfilename);
	logfile.open(logfilepath.c_str());
	
	logfile << "Defect Amplitude=" << defectAmplitude << endl;
	logfile << "Mean Y=" << ymean << endl;
	
	fillPage(100);
	addFiducials();
	
	// defect
	vnext= defectTop;
	long pw=0,ph=0;
	double defectWidth= 0;
	double defectHeight= 0;
	if (includeDefect) { 
		addDefectImage(&pw,&ph,ymean,defectAmplitude);
		defectWidth= pw * fPage->dx();
		defectHeight= ph * fPage->dy();
		//cout << "Defect image dimensions: " << pw << " by " << ph << endl;
	} else {
		// this implementation is a hack - should not use hardcoded dimensions:
		pw= 2047;
		ph= 2047;
		defectWidth= pw * fPage->dx();
		defectHeight= ph * fPage->dy();
		setFlatField(defectLeft,defectTop,defectWidth,defectHeight,ymean);
	}
	vnext += defectHeight + fElMargin;
	
	// TRC
	double trcHeight= 8;
	addTrc(defectLeft,vnext,defectWidth,trcHeight,fYMinimum-2.0,fYMaximum+2.0,20);
	vnext += trcHeight + fElMargin;
	addTrc(defectLeft,vnext,defectWidth,trcHeight,0.0,100.0,20);
	vnext += trcHeight + fElMargin;
	
	// Vertical bands, scaled according to defect:
	lnext= defectLeft;
	double regionWidth= (defectWidth - 2*fElMargin) / 3; 
	addVerticalBands(lnext,vnext,regionWidth,bandLength,defectAmplitude, 2.0);
	lnext += regionWidth + fElMargin;
	addVerticalBands(lnext,vnext,regionWidth,bandLength,defectAmplitude, 1.0);
	lnext += regionWidth + fElMargin;
	addVerticalBands(lnext,vnext,regionWidth,bandLength,defectAmplitude, 0.2);
	vnext += bandLength + fElMargin;
	lnext += regionWidth + fElMargin;
	
	// Vertical bands, constant amplitude:
	double constAmplitude= 1.0;
	lnext= defectLeft;
	regionWidth= (defectWidth - 2*fElMargin) / 3; 
	addVerticalBands(lnext,vnext,regionWidth,bandLength,constAmplitude, 2.0);
	lnext += regionWidth + fElMargin;
	addVerticalBands(lnext,vnext,regionWidth,bandLength,constAmplitude, 1.0);
	lnext += regionWidth + fElMargin;
	addVerticalBands(lnext,vnext,regionWidth,bandLength,constAmplitude, 0.2);
	vnext += bandLength + fElMargin;
	lnext += regionWidth + fElMargin;
	
	// Uniform horizontal strips:
	double smallUniformHeight= 6.0;
	double largeUniformHeight= 20;
	vnext= defectTop - largeUniformHeight - 2*smallUniformHeight - 3*fElMargin;
	addUniformGray(defectLeft,vnext,defectWidth,smallUniformHeight, rmsOffsetLevel(-2.0));
	logfile << "Uniform\t" << rmsOffsetLevel(-2.0) << endl;
	vnext += smallUniformHeight + fElMargin;
	addUniformGray(defectLeft,vnext,defectWidth,smallUniformHeight, rmsOffsetLevel(2.0));
	logfile << "Uniform\t" << rmsOffsetLevel(2.0) << endl;
	vnext += smallUniformHeight + fElMargin;
	addUniformGray(defectLeft,vnext,defectWidth,largeUniformHeight, rmsOffsetLevel(0.0));
	logfile << "Uniform\t" << rmsOffsetLevel(0.0) << endl;
	vnext += smallUniformHeight + fElMargin;
	
	// Horizontal bands:
	vnext= defectTop;
	lnext= defectLeft - fElMargin - bandLength;
	double regionHeight= (defectHeight - 2*fElMargin) / 3;
	addHorizontalBands(lnext,vnext,bandLength,regionHeight,defectAmplitude, 0.2);
	vnext += regionHeight + fElMargin;
	addHorizontalBands(lnext,vnext,bandLength,regionHeight,defectAmplitude, 1.0);
	vnext += regionHeight + fElMargin;
	addHorizontalBands(lnext,vnext,bandLength,regionHeight,defectAmplitude, 2.0);
	vnext += regionHeight + fElMargin;
	
	// Uniform vertical strips:
	vnext= defectTop;
	lnext= defectLeft + defectWidth + fElMargin;
	addUniformGray(lnext,vnext,smallUniformHeight,defectHeight, rmsOffsetLevel(-2.0));
	logfile << "Uniform\t" << rmsOffsetLevel(-2.0) << endl;
	lnext += smallUniformHeight + fElMargin;
	addUniformGray(lnext,vnext,smallUniformHeight,defectHeight, rmsOffsetLevel(2.0));
	logfile << "Uniform\t" << rmsOffsetLevel(2.0) << endl;
	lnext += smallUniformHeight + fElMargin;
	addUniformGray(lnext,vnext,smallUniformHeight,defectHeight, rmsOffsetLevel(0.0));
	logfile << "Uniform\t" << rmsOffsetLevel(0.0) << endl;
	lnext += smallUniformHeight + fElMargin;
	
}

void w11MacroScaleSample::simulatedVerticalBands(double a, double fcpmm)
{
	addVerticalBands(fDefectLeft, fDefectTop ,fDefectWidth, fDefectHeight, a, fcpmm, false);
}

void w11MacroScaleSample::simulatedHorizontalBands(double a, double fcpmm)
{
	addHorizontalBands(fDefectLeft, fDefectTop ,fDefectWidth, fDefectHeight, a, fcpmm, false);
}
void w11MacroScaleSample::fillPage(void)
{
	fillPage((float)fYMean);
}

void w11MacroScaleSample::fillPage(float val)
{
	long iy,ix;
	float *fptr= NULL;
	
	for (iy=0;iy < fPage->ny();iy++) {
		fptr= fPage->rowPtr(iy);
		for (ix=0;ix < fPage->nx();ix++) {
			*fptr++ = val;
		}
	}
}

void w11MacroScaleSample::setFlatField(double left,double top,double width,double height,double value)
{
	long pxstart= static_cast<long>(left / fPage->dx());
	long pystart= static_cast<long>(top / fPage->dy());
	long pxend= static_cast<long>(pxstart + width / fPage->dx());
	long pyend= static_cast<long>(pystart + height / fPage->dy());
	long pxwidth= pxend - pxstart + 1;
	
	for (long iy=pystart; iy < pyend; iy++) {
		float *rptr= fPage->rowPtr(iy);
		rptr += pxstart;
		for (long ix= 0;ix < pxwidth; ix++) {
			*rptr++ = (float) value;
		}
	}
	outlineRegion(left,top,width,height,100);
}

void w11MacroScaleSample::copyToDefectRegion(const w11ImageChannelT<float> &image)
{
	long ystart= static_cast<long>(fDefectTop / fPage->dy());
	long xstart= static_cast<long>(fDefectLeft / fPage->dx());
	for (long iy=0;iy < image.ny(); iy++) {
		const float *sptr= image.constRowPtr(iy);
		float *dptr= fPage->rowPtr(ystart+iy);
		dptr += xstart;
		for (long ix=xstart;ix < xstart + image.nx(); ix++) {
			*dptr++ = *sptr++; 
		}
	}
	outlineRegion(fDefectLeft, fDefectTop, image.width(), image.height(), 100);
}

void w11MacroScaleSample::addDefectImage(long *pwidth,long *pheight,double meanY,double amplitude)
{
	w11MacroScale   fScale;
	
	w11ImageChannelT<float> *image= fScale.makeDefectImage(meanY, amplitude);
	
	image->getStat1(fYMean, fYRms);
	image->getRange(fYMinimum, fYMaximum);
	
	cout << "Minimum = " << fYMinimum << endl;
	cout << "Mean =    " << fYMean << endl;
	cout << "Maximum = " << fYMaximum << endl;
	cout << "RMS =     " << fYRms << endl;
	
	// copy into fPage:
	this->copyToDefectRegion(*image);		// input parameters left and top no longer used - using data members instead
	//outlineRegion(fDefectLeft, fDefectTop, image->width(), image->height(), 100);
	
	*pwidth= image->nx();
	*pheight= image->ny();
}

void w11MacroScaleSample::addTrc(double left,double top,double width,double height,double ystart,double yend,long nlevels)
{
	double patchWidth= (width - (nlevels-1)*fElMargin) / nlevels;
	double lnext=0;
	double dy= (yend - ystart) / (nlevels-1);
	double value;
	
	lnext= left;
	for (long k=0;k<nlevels;k++) {
		value= ystart+k*dy;
		addUniformGray(lnext, top, patchWidth, height, value);
		logfile << "TRC\t" << k << "\t" << value << endl;
		lnext += patchWidth + fElMargin;
	}
}

void w11MacroScaleSample::addVerticalBands(double left,double top,double width,double height, 
										   double a, double fcpmm, bool replace)
{
	long pxstart= static_cast<long>(left / fPage->dx());
	long pystart= static_cast<long>(top / fPage->dy());
	long pxend= static_cast<long>(pxstart + width / fPage->dx());
	long pyend= static_cast<long>(pystart + height / fPage->dy());
	long pxwidth= pxend - pxstart + 1;
	double dx= fPage->dx();
	double x= 0;
	const double twopi= 2 * 3.1415926535898;
	
	for (long iy=pystart; iy < pyend; iy++) {
		float *rptr= fPage->rowPtr(iy);
		rptr += pxstart;
		x= 0;
		for (long ix= 0;ix < pxwidth; ix++) {
			if (replace) {
				*rptr++ = (float) (fYMean + a * cos(twopi * x * fcpmm));
			} else {
				*rptr++ +=  (float) (a * cos(twopi * x * fcpmm));
			}
			x += dx;
		}
	}
	outlineRegion(left,top,width,height,100);
}

void w11MacroScaleSample::addHorizontalBands(double left,double top,double width,double height,
											 double a, double fcpmm, bool replace)
{
	long pxstart= static_cast<long>(left / fPage->dx());
	long pystart= static_cast<long>(top / fPage->dy());
	long pxend= static_cast<long>(pxstart + width / fPage->dx());
	long pyend= static_cast<long>(pystart + height / fPage->dy());
	long pxwidth= pxend - pxstart + 1;
	double dy= fPage->dy();
	double y= 0;
	const double twopi= 2 * 3.1415926535898;
	double val= 0;
	
	for (long iy=pystart; iy < pyend; iy++) {
		float *rptr= fPage->rowPtr(iy);
		val= fYMean + a * cos(twopi * y * fcpmm);
		rptr += pxstart;
		y += dy;
		for (long ix= 0;ix < pxwidth; ix++) {
			if (replace) {
				*rptr++ = (float) val;
			} else {
				*rptr++ += (float) val;
			}
		}
	}
	outlineRegion(left,top,width,height,100);
}

void w11MacroScaleSample::addUniformGray(double left,double top,double width,double height,double yvalue)
{
	long pxstart= static_cast<long>(left / fPage->dx());
	long pystart= static_cast<long>(top / fPage->dy());
	long pxend= static_cast<long>(pxstart + width / fPage->dx());
	long pyend= static_cast<long>(pystart + height / fPage->dy());
	long pxwidth= pxend - pxstart + 1;
	
	for (long iy=pystart; iy < pyend; iy++) {
		float *rptr= fPage->rowPtr(iy);
		rptr += pxstart;
		for (long ix= 0;ix < pxwidth; ix++) {
			*rptr++ = (float) yvalue;
		}
	}
	
	outlineRegion(left,top,width,height,100);
}

void w11MacroScaleSample::outlineRegion(double left,double top,double width,double height,double rimvalue)
{
	return;
	
	long pxstart= static_cast<long>(left / fPage->dx() + 0.5);
	long pystart= static_cast<long>(top / fPage->dy() + 0.5);
	long pxend= static_cast<long>(pxstart + width / fPage->dx() + 0.5);
	long pyend= static_cast<long>(pystart + height / fPage->dy() + 0.5);
	long pxwidth= pxend - pxstart + 1;
	
	float *rptr= fPage->rowPtr(pystart);
	rptr += pxstart;
	for (long ix= 0;ix < pxwidth; ix++) {
		*rptr++ = (float) rimvalue;
	}
	rptr= fPage->rowPtr(pyend-1);
	rptr += pxstart;
	for (long ix= 0;ix < pxwidth; ix++) {
		*rptr++ = (float) rimvalue;
	}
	for (long iy=pystart; iy < pyend; iy++) {
		rptr= fPage->rowPtr(iy);
		rptr += pxstart;
		*rptr++ = (float) rimvalue;
		rptr += pxwidth;
		*rptr++ = (float) rimvalue;
	}
}



void w11MacroScaleSample::addFiducials(void)
{
	double x0= 12;
	double x1= fPage->width() - x0;
	double y0= 30;
	double y1= fPage->height() - y0;
	
	makeMark(x0,y0);
	makeMark(x1,y0);
	makeMark(x1,y1);
	makeMark(x0,y1);
}

double w11MacroScaleSample::rmsOffsetLevel(double frms)
{
	return fYMean + frms * fYRms;
}

void w11MacroScaleSample::makeMark(double x,double y)
{
	double mmsize= 1.5;
	long pxsize= static_cast<long>(mmsize / fPage->dx());
	long pysize= static_cast<long>(mmsize / fPage->dy());
	long xstart= static_cast<long>(x / fPage->dx() - pxsize/2);
	long ystart= static_cast<long>(y / fPage->dy() - pysize/2);
	float mvalue= 0;
	long ymargin= pysize;
	long xmargin= pxsize;
	
	mvalue= 100;
	for (long iy=ystart-ymargin;iy<ystart+ymargin+pysize;iy++) {
		for (long ix=xstart-xmargin;ix<xstart+xmargin+pxsize;ix++) {
			(*fPage)(iy,ix)= mvalue;
		}
	}     
	
	mvalue= 0;
	for (long iy=ystart;iy<ystart+pysize;iy++) {
		for (long ix=xstart;ix<xstart+pxsize;ix++) {
			(*fPage)(iy,ix)= mvalue;
		}
	}   
}






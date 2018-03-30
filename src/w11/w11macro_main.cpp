/*
 *  w11macro_main.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/7/08.
 *
 */

#include "w11macro_main.h"
#include "w11Build.h"
#include "w11Utilities.h"
#include "w11Defects.h"
#include "w11MacroScaleSample.h"
#include "w11CalibrationTP.h"
#include "w11ImageText.h"
#include "w11profile.h"
#include "w11Mio.h"

#include <tclap/CmdLine.h>
using namespace TCLAP;

#include <vector>
#include <string>
#include <time.h>
using namespace std;

typedef enum {
	TScaleOp,
	TCalibrationOp,
	TAnalysisOp,
	TTpCalibrationOp,
	TTpSimulationOp
} TOperationMode;

void processMioFile(string miofile);
void scaleOp(float ymean, float amplitude, float noise, const string& label);
void scaleOp2(exmRequest& req);
void calibrationOp(vector<string> inputFileNames);
void calibrationOp2(exmRequest& req);
void analysisOp(void);
void analysisOp2(exmRequest& req);
void calibrationTestPatternOp(void);
void calibrationTestPatternOp2(exmRequest& req);
void simulationOp(string filename, string label,float mean,float noise,
				  vector<double> vbandsf, vector<double> vbandsa,
				  vector<double> hbandsf, vector<double> hbandsa,
				  vector<double> simMottleLowFreqs, vector<double> simMottleHighFreqs, vector<double> simMottleAmplitudes,
				  double rsAmplitude, double rsMinPeriod, double rsMaxPeriod,
				  vector<double> simVStreakLocations,vector<double> simVStreakWidths, vector<double> simVStreakAmplitudes);
void simulationOp2(exmRequest& req);
static const double gSampleWidth = 210.0; 
static const double gSampleHeight = 279.0; 
static const double gSampleDpi = 300.0; 


int main(int argc,char **argv)
{
	try {
		//////////////////////////////////////////////////////////////////////////////////////////
		// parse the command line (using TCLAP - <http://tclap.sourceforge.net/>):
		//
		string documentation= \
		"INCITS W1.1 macro-uniformity tool. The tool can be used for multiple purposes:\n\
		- Monochrome printer calibration;\n\
		- Generation of macro-uniformity ruler samples;\n\
		- Simulation of simple defects (banding);\n\
		- Imposition of simple defects (not yet implemeted)\n\
		\n\
		You must give one argument on the command line, which is the path to a file that contains all input parameters. For example, enter:\n\n\
		w11macro.exe --mio c:\\w11macro\\testdirectory\\inputfiles\\test1.mio \n\
		";
		
		CmdLine cmd(documentation, ' ',	__W11_MACRO_VERSION__);				// delimiter and version
		
		ValueArg<string> mioArg("", "mio", "Module I/O file", true, "", "string");
		cmd.add(mioArg);
		
		// parse: ===========================================================================
		cmd.parse(argc, argv);
		
		string miofile= mioArg.getValue();
		if (miofile.length() > 0) {
			processMioFile(miofile);
		} else {
			throw string("mio file name is empty");
		}

	}
	catch (ArgException &e) { // catch exceptions from TCLAP
		cerr << "TCLAP error: " << e.error() << " for arg " << e.argId() << endl;
		return 1;
	}
	catch (string &e) {
		cout << "Error: " << e << endl;
		cerr << "Error: " << e << endl;
		return 1;
	}
	catch (...) {
		cerr << "main: Uncaught exception." << endl;
		return 1;
	}
	return 0;
}

void processMioFile(string miofile)
{
	w11Mio exm;
	exm.read(miofile.c_str());
	
	vector<string> reqids= exm.requestIdList();
	vector<string>::iterator sit;
	
	for (sit=reqids.begin(); sit != reqids.end(); ++sit) {
		exmRequest req= exm.selectRequestId(*sit);
		string id= *sit;
		string imagepath= req.imagePath();
		
		string homedir= req.inputStringForKey("Directory");
		w11Utility::instance().setHomeDirectory(homedir);
		
		string smode= req.exmIdentifier();
		if (smode == "scale") {
			scaleOp2(req);
		} else if (smode == "calibration") {
			calibrationOp2(req);
		} else if (smode == "analysis") {
			analysisOp2(req);
		} else if (smode == "tp_calibration") {
			calibrationTestPatternOp2(req);
		} else if (smode == "simulation") {
			simulationOp2(req);
		} else {
			throw string("Internal error: Invalid operation mode");
		}
		
		/*
		 
		cout << "Inputs:" << endl;
		vector<exmParameter*> inputs= req.inputs();
		vector<exmParameter*>::iterator dit;
		for (dit=inputs.begin(); dit != inputs.end(); ++dit) {
			(*dit)->write(stdout);
		}
		
		cout << "Outputs:" << endl;
		vector<exmParameter*> outputs= req.outputs();
		for (dit=outputs.begin(); dit != outputs.end(); ++dit) {
			(*dit)->write(stdout);
		}
		 */
		
	}
	
}

void scaleOp(float ymean, float amplitude, float noise, const string& label)
{
	w11MacroScaleSample sample(gSampleWidth, gSampleHeight, (int) gSampleDpi);
	
	// construct file name:
	ostringstream ost;
	ost << "w11macro_scale_a=" << static_cast<long>(10000*amplitude) << "_n=" << static_cast<long>(100*noise);
	string scalePath= w11Utility::instance().generatedScaleDirectory();
	scalePath= w11Utility::appendPathSegment(scalePath, ost.str() + ".tif");
	
	//
	sample.compose(ymean, amplitude, ost.str());
	
	// prep for TRC calibration
	string calibrationPath= w11Utility::instance().generatedCalibrationDirectory();
	calibrationPath= w11Utility::appendPathSegment(calibrationPath, "w11_calibration.cal");
	w11CalibrationProcess cp;
	cp.loadCalibrationData(calibrationPath);
	
	cout << "Applying calibration..." << endl;
	sample.calibrate(cp);
	
	// quantize
	w11ImageChannelT<w11T8Bits> *i8= quantize(sample.image(),0.0,100.0, noise);
	
	{ // add label to image
		string fname= w11Utility::fontName();
		w11ImageText it(w11Utility::instance().fontDirectory(), fname);
		
		{ // mark TP by date, time, amplitude, version
			time_t rawtime;
			time(&rawtime);
			struct tm * timeinfo= localtime(&rawtime);
			char datetime[256];
			strftime(datetime, 255, "%Y.%m.%d.%H.%M.%S", timeinfo);
			
			ostringstream ost;
			//ost << asctime(timeinfo);
			ost << datetime;
			ost << setiosflags( ios::fixed ) << setiosflags( ios::showpos )  << setprecision(9);
			ost << " A=" << setw(7) << setprecision(5) << amplitude << " N=" << noise << " Version:" << __W11_MACRO_VERSION__;
			ost << "---" << label;
			
			it.setText(ost.str());
			it.addTextAt(*i8, 18.0, 12.0);
		}
	}
	
	//i8->writeFile(scalePath);
	float xdpi = (float) (25.4 / i8->dx());
	float ydpi = (float) (25.4 / i8->dy());
	writeTiff(i8->matrix(), scalePath, xdpi, ydpi, string("W1.1 Macro-uniformity") + label);
	cout << "Wrote TIFF image <" << scalePath + ">" << endl;
	cout << "Dimensions: " << i8->nx() << " by " << i8->ny() << endl;
}

void scaleOp2(exmRequest& req)
{
	float ymean= (float) req.inputRealForKey("Mean");
	float amplitude= (float) req.inputRealForKey("Amplitude");
	float noise= (float) req.inputRealForKey("Noise");
	string label= req.inputStringForKey("Label");
	
	scaleOp(ymean, amplitude, noise, label);
}


void calibrationOp(vector<string> inputFileNames)
{
	string outputCalibrationPath= w11Utility::instance().generatedCalibrationDirectory();
	outputCalibrationPath= w11Utility::appendPathSegment(outputCalibrationPath, "w11_calibration.cal");
	
	w11CalibrationProcess cp;	
	string inputCalibrationDir= w11Utility::instance().calibrationInputDirectory();
	string inputCinPath= w11Utility::appendPathSegment(inputCalibrationDir, "w11_calibration_cin.data");
	vector<string>::iterator sit;
	cout << "Creating calibration based on " << inputFileNames.size() << " data file(s)." << endl;
	for (sit=inputFileNames.begin(); sit != inputFileNames.end(); ++sit) {
		cout << "Data file: " << *sit << endl;
		string inputCalibrationPath= w11Utility::appendPathSegment(inputCalibrationDir, *sit);
		cp.addCalibrationData(inputCinPath, inputCalibrationPath);
	}
	cp.generateCalibration();
	cp.storeCalibrationData(outputCalibrationPath);
	
#if 1
	// testing
	cp.loadCalibrationData(outputCalibrationPath);
	cout << "Testing calibration:" << endl;
	for (double x=0; x <= 100; x += 5.0) {
		try {
			cout << x << " ===> ";
			double y= cp.calibrate(x);
			cout << y << endl;
		}
		catch (string &e) {
			cout << e << endl;
		}
	}
#endif
}

void calibrationOp2(exmRequest& req)
{
	vector<string> calFileNames= req.inputStringArrayForKey("CIELabFiles");
	calibrationOp(calFileNames);
}


void analysisOp(void)
{
	cout << "Analysis not yet implemented" << endl;
}

void analysisOp2(exmRequest& req)
{
	cout << "Analysis not yet implemented" << endl;
}

void calibrationTestPatternOp(void)
{
	w11TestPattern tp;
	tp.addPatches();
	w11ImageChannelT<w11T16Bits> *image= tp.paint();
	w11ImageChannelT<w11T8Bits> *i8= quantize(*image);
	ostringstream ost;
	ost << "calib_x" << i8->nx() << "_y" << i8->ny(); 
	
	string fname= w11Utility::fontName();
	w11ImageText it(w11Utility::instance().fontDirectory(), fname);
	
	{ // mark TP by date and time
		time_t rawtime;
		time(&rawtime);
		struct tm * timeinfo= localtime(&rawtime);
		char datetime[256];
		strftime(datetime, 255, "%Y.%m.%d.%H.%M.%S", timeinfo);
		
		ostringstream ost;
		ost << datetime;
		
		it.setText(ost.str());
		it.addTextAt(*i8, 20.0, 16.0);
	}
	string calibrationPath= w11Utility::instance().generatedCalibrationDirectory();
	calibrationPath= w11Utility::appendPathSegment(calibrationPath, ost.str());
	
	float xdpi = (float)(25.4 / i8->dx());
	float ydpi = (float)(25.4 / i8->dy());
	writeTiff(i8->matrix(), calibrationPath + ".tif", xdpi, ydpi, string("W1.1 Macro-uniformity") + __W11_MACRO_VERSION__);
	cout << "Wrote TIFF image <" << calibrationPath + ".tif" + ">" << endl;
	
	//i8->writeFile(calibrationPath);
}

void calibrationTestPatternOp2(exmRequest& req)
{
	calibrationTestPatternOp();
}

void simulationOp(string filename, string label, float mean, float noise,
				  vector<double> vbandsf, vector<double> vbandsa,
				  vector<double> hbandsf, vector<double> hbandsa,
				  vector<double> simMottleLowFreqs, vector<double> simMottleHighFreqs, vector<double> simMottleAmplitudes,
				  double rsAmplitude, double rsMinPeriod, double rsMaxPeriod,
				  vector<double> simVStreakLocations,vector<double> simVStreakWidths, vector<double> simVStreakAmplitudes)
{
	cout << "Creating simulated defects" << endl;
	
	// build the defect image:
	w11MacroScale   fScale;
	w11Banding *bs= NULL;
	w11Mottle *mottle= NULL;
	w11RandomStreaks *rs= NULL;
	w11IsolatedStreaks *streak=NULL;
	double dxy= 25.4 / DPI;
	ostringstream ost;
	
	// vertical bands:
	for (long i=0; i<vbandsf.size(); i++) {
		double lambda= 1.0 / vbandsf[i];
		bs= new w11Banding(vbandsa[i], 0, dxy, lambda, 0, true);
		fScale.addDefect(bs);
		cout << "Added defect: " << bs->string() << endl;
	}
	
	// horizontal bands:
	for (long i=0; i<hbandsf.size(); i++) {
		double lambda= 1.0 / hbandsf[i];
		bs= new w11Banding(hbandsa[i], 0, dxy, lambda, 0, false);
		fScale.addDefect(bs);
		cout << "Added defect: " << bs->string() << endl;
	}
	
	// mottle
	for (long i=0; i<simMottleAmplitudes.size(); i++) {
		double contrast= simMottleAmplitudes[i];
		double lowf= simMottleLowFreqs[i];
		double highf= simMottleHighFreqs[i];
		mottle= new w11Mottle(contrast, 1001, dxy, lowf, highf);
		fScale.addDefect(mottle);
		cout << "Added defect: " << mottle->string() << endl;
	}
	
	// vertical random streaks
	if (rsAmplitude > 0) {
		long rsSeed= 31415;
		rs= new w11RandomStreaks(rsAmplitude,rsSeed,dxy,rsMinPeriod,rsMaxPeriod, false);	// bug in w11RandomStreaks means vertical / horizontal are swapped
		fScale.addDefect(rs);
		cout << "Added defect: " << rs->string() << endl;
		ost << "rs_amp\t" << rsAmplitude << "\t";
	}
	
	
	// isolated vertical streaks
	for (long i=0; i<simVStreakLocations.size(); i++) {
		double amplitude= simVStreakAmplitudes[i];
		double location= simVStreakLocations[i];
		double width= simVStreakWidths[i];
		bool bright= false;
		streak= new w11IsolatedStreaks(amplitude,0,dxy,location, width,bright,true);
		fScale.addDefect(streak);
		cout << "Added defect: " << streak->string() << endl;
		ost << "is\t" << amplitude << "\t";
	}
	
	
	
	// render the defect image:
	w11ImageChannelT<float> *image= fScale.generateFloat(IMAGE_SIZE,IMAGE_SIZE,mean,DPI);
	
	// put the defect image into a full sample: 
	w11MacroScaleSample sample(gSampleWidth, gSampleHeight, (int)gSampleDpi);
	sample.compose(mean, 0.0, label, false);
	sample.copyToDefectRegion(*image);
	
	// construct file name:
	string scalePath= w11Utility::instance().generatedScaleDirectory();
	scalePath= w11Utility::appendPathSegment(scalePath, filename);
	
	// TRC calibration
	string calibrationPath= w11Utility::instance().generatedCalibrationDirectory();
	calibrationPath= w11Utility::appendPathSegment(calibrationPath, "w11_calibration.cal");
	w11CalibrationProcess cp;
	cp.loadCalibrationData(calibrationPath);
	sample.calibrate(cp);
	
	w11ImageChannelT<w11T8Bits> *i8= quantize(sample.image(),0.0,100.0, noise);
	
	{ // add label to image
		string fname= w11Utility::fontName();
		w11ImageText it(w11Utility::instance().fontDirectory(), fname);
		
		{ // mark TP by date, time, amplitude, version
			time_t rawtime;
			time(&rawtime);
			struct tm * timeinfo= localtime(&rawtime);
			char datetime[256];
			strftime(datetime, 255, "%Y.%m.%d.%H.%M.%S", timeinfo);
			
			ostringstream ost;
			//ost << asctime(timeinfo);
			ost << datetime;
			ost << setiosflags( ios::fixed ) << setiosflags( ios::showpos )  << setprecision(9);
			ost << "---" << label;
			
			it.setText(ost.str());
			it.addTextAt(*i8, 18.0, 12.0);
		}
	}
	
	float xdpi = (float)(25.4 / i8->dx());
	float ydpi = (float)(25.4 / i8->dy());
	writeTiff(i8->matrix(), scalePath, xdpi, ydpi, string("W1.1 Macro-uniformity") + label);
	cout << "Generated simulated image: " << scalePath << endl;
	cout << "Dimensions: " << i8->nx() << " by " << i8->ny() << endl;
	
#ifdef DRR_TP // DRR debug only
#include "drr_tp_profile.h"
#endif
	
}

void simulationOp2(exmRequest& req)
{

	cout << "Creating simulated defects" << endl;

	float ymean= (float)req.inputRealForKey("Mean");
	float noise= (float)req.inputRealForKey("Noise");
	string label= req.inputStringForKey("Label");
	string simFileName= req.inputStringForKey("Filename");
	
	// banding:
	vector<double> simVbandsFrequency= req.inputDoubleArrayForKey("VerticalBandsFrequencies");	// what if there are none?
	vector<double> simVbandsAmplitude= req.inputDoubleArrayForKey("VerticalBandsAmplitudes");
	vector<double> simHbandsFrequency= req.inputDoubleArrayForKey("HorizontalBandsFrequencies");
	vector<double> simHbandsAmplitude= req.inputDoubleArrayForKey("HorizontalBandsAmplitudes");

	// mottle:
	vector<double> simMottleLowFreqs= req.inputDoubleArrayForKey("MottleLowFrequencies");
	vector<double> simMottleHighFreqs= req.inputDoubleArrayForKey("MottleHighFrequencies");
	vector<double> simMottleAmplitudes= req.inputDoubleArrayForKey("MottleAmplitudes");

	// random streaking:
	double vrsAmplitude= req.inputRealForKey("VerticalRandomStreakAmplitude");
	double vrsMinPeriod= req.inputRealForKey("VerticalRandomStreakMinimumPeriod");
	double vrsMaxPeriod= req.inputRealForKey("VerticalRandomStreakMaximumPeriod");
	
	// isolated streaks:
	vector<double> simVStreakLocations= req.inputDoubleArrayForKey("VerticalIsolatedStreakLocations");
	vector<double> simVStreakWidths= req.inputDoubleArrayForKey("VerticalIsolatedStreakWidths");
	vector<double> simVStreakAmplitudes= req.inputDoubleArrayForKey("VerticalIsolatedStreakAmplitudes");
	
	simulationOp(simFileName,label,ymean,noise,
				 simVbandsFrequency, simVbandsAmplitude, 
				 simHbandsFrequency, simHbandsAmplitude,
				 simMottleLowFreqs, simMottleHighFreqs, simMottleAmplitudes,
				 vrsAmplitude,vrsMinPeriod,vrsMaxPeriod,
				 simVStreakLocations,simVStreakWidths,simVStreakAmplitudes);
}






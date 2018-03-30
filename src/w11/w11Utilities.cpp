/*
 *  w11Utilities.cpp
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/7/08.
 *
 */



#include "w11Utilities.h"
using namespace std;

/*
std::string w11HomeDirectory(void)
{
	std::string homepath;
#ifdef __windows__
	
	// consider getting the path from the command line interpreter, which probably can return the exe path.
	
	char mydirectory[MAX_PATH] = {""};
	GetCurrentDirectory(MAX_PATH,mydirectory);
	homepath= mydirectory;
#endif
	return homepath;
	
}
 */


w11Utility* w11Utility::sInstance= 0;

string w11Utility::filePathSeparator(void)
{
#ifdef WIN32
	return "\\";
#else
	return "/";		// UNIX
#endif
}

string w11Utility::appendPathSegment(const string& base, const string& segment)
{
	string newpath= base + filePathSeparator() + segment;
	return newpath;
}

w11Utility& w11Utility::instance(void)
{
	if (sInstance == 0) {
		sInstance= new w11Utility;
	}
	return *sInstance;
}

string w11Utility::fontName(void)
{
#ifdef WIN32
	return("arial.ttf");
#else
	return("Arial");
#endif
}


w11Utility::w11Utility(void)
{
	
}

w11Utility::~w11Utility(void)
{
	
}

void w11Utility::setHomeDirectory(const std::string path)
{
	fHomeDirectory= path;
}


std::string w11Utility::homeDirectory(void) const
{
	return fHomeDirectory;
}

string w11Utility::generatedScaleDirectory(void) const
{
	string filepath= homeDirectory();
	filepath= appendPathSegment(filepath, "generated_scales");
	return filepath;
}

string w11Utility::generatedCalibrationDirectory(void) const
{
	string filepath= homeDirectory();
	filepath= appendPathSegment(filepath, "generated_calibrations");
	return filepath;
}

string w11Utility::calibrationInputDirectory(void) const
{
	string filepath= homeDirectory();
	filepath= appendPathSegment(filepath, "calibration_input");
	return filepath;
}

string w11Utility::fontDirectory(void) const
{
	string filepath= homeDirectory();
	filepath= appendPathSegment(filepath, "fonts");
	return filepath;
}




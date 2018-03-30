/*
 *  w11Utilities.h
 *  w11macro
 *
 *  Created by Rene Rasmussen on 9/7/08.
 *
 */

#ifndef __w11_utilities__
#define __w11_utilities__

#include <string>

inline double w11min(const double a, const double b)
{
	return a < b ? a : b;
}

inline double w11max(const double a, const double b)
{
	return a > b ? a : b;
}

inline float w11min(const float a, const float b)
{
	return a < b ? a : b;
}

inline float w11max(const float a, const float b)
{
	return a > b ? a : b;
}

class w11Utility {	// Singleton pattern
public:
	static w11Utility& instance(void);
	static std::string filePathSeparator(void);
	static std::string appendPathSegment(const std::string& base, const std::string& segment);
	static std::string fontName(void);
	
	void setHomeDirectory(const std::string path);
	std::string homeDirectory(void) const;
	std::string generatedScaleDirectory(void) const;
	std::string generatedCalibrationDirectory(void) const;
	std::string calibrationInputDirectory(void) const;
	std::string fontDirectory(void) const;
	
protected:
	w11Utility(void);
	virtual ~w11Utility();
	
private:
	static w11Utility*	sInstance;
	std::string fHomeDirectory;
	
	
};

#endif



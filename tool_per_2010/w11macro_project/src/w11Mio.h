/*
 *  w11Mio.h
 *
 *  Facility to make it easier to specify input parameters to the executable for image simulation of scales etc., and to obtain the calculated parameters (output)
 *	for verification of scales.
 *
 *  Created by Rene Rasmussen on 8/13/09 for INCITS W1.1 Macro-uniformity.
 *
 */

#include <string>
#include <map>
#include <vector>
#include <stdio.h>

typedef enum {
	exm_t_unknown= 0,	// 0	"00"
	exm_t_string,		// 1	"SS" equivalent 2-character code. Second letter is for Scalar or Array
	exm_t_long,			// 2	"LS"
	exm_t_float,		// 3	"FS"
	exm_t_double,		// 4	"DS"
	exm_t_string_array,	// 5	"SA"
	exm_t_float_array,	// 6	"FA"
	exm_t_invalid		// 7	"99"
} TExmDataType;

class exmData {
public:
	virtual void read(FILE *f) = 0;
	virtual void write(FILE *f) const = 0;
	
	virtual TExmDataType type(void) const = 0;
};

class exmParameter {
public:
	exmParameter(std::string _name, TExmDataType _type);
	exmParameter(std::string _name, exmData *_ed);
	exmParameter(FILE *f);
	void init_exmParameter(std::string _name, TExmDataType _type);
	virtual ~exmParameter() {};
	std::string name(void) {
		return fName;
	};
	TExmDataType type(void) {
		return dtype;
	};
	exmData* data(void) {return fdata;};
	void replaceData(exmData *_ed);
	virtual void write(FILE *f) const;
	
protected:
	TExmDataType dtype;
	std::string fName;
private:
	exmData *fdata;
	
	TExmDataType typeFromCharCode(const char ch1,const char ch2) const;
	std::string charCodeFromType(const TExmDataType ptype) const;

};

class exmString : public exmData {
public:
	exmString(void) {};
	exmString(std::string _value);
	virtual ~exmString() {};
	TExmDataType type(void) const {
		return exm_t_string;
	};
	std::string value(void) {return fValue;};
	virtual void read(FILE *f);
	virtual void write(FILE *f) const;
private:
	std::string fValue;
};

class exmLong : public exmData {
public:
	exmLong(void) {};
	exmLong(long _value);
	TExmDataType type(void) const {
		return exm_t_long;
	};
	long value(void) {return fValue;};
	virtual void read(FILE *f);
	virtual void write(FILE *f) const;
private:
	long fValue;
};

class exmFloat : public exmData {
public:
	exmFloat(void) {};
	exmFloat(float _value);
	TExmDataType type(void) const {
		return exm_t_float;
	};
	float value(void) {return fValue;};
	virtual void read(FILE *f);
	virtual void write(FILE *f) const;
private:
	float fValue;
};

class exmDouble : public exmData {
public:
	exmDouble(void) {};
	exmDouble(double _value);
	TExmDataType type(void) const {
		return exm_t_double;
	};
	double value(void) {return fValue;};
	virtual void read(FILE *f);
	virtual void write(FILE *f) const;
private:
	double fValue;
};

class exmFloatArray : public exmData {
public:
	exmFloatArray(void) {};
	exmFloatArray(float *values, long n);
	TExmDataType type(void) const {
		return exm_t_float_array;
	};
	std::vector<float>& value(void) {return fValue;};
	virtual void read(FILE *f);
	virtual void write(FILE *f) const;
private:
	std::vector<float> fValue;
};

class exmStringArray : public exmData {
public:
	exmStringArray(void) {};
	exmStringArray(std::vector<std::string> values);
	TExmDataType type(void) const {
		return exm_t_string_array;
	};
	std::vector<std::string>& value(void) {return fValue;};
	virtual void read(FILE *f);
	virtual void write(FILE *f) const;
private:
	std::vector<std::string> fValue;
};


class exmRequest {
public:
	exmRequest(std::string _uuid) {
		fuuid= _uuid;
		fcompleted= false;
	};
	
	// file io
	void read(FILE *f);
	void write(FILE *f);
	
	bool isCompleted(void) {
		return fcompleted;
	};
	std::string uuid(void) {
		return fuuid;
	};
	
	// external module:
	void setExmIdentifier(std::string exmid);		/* name or path of the external module (executable) */
	std::string exmIdentifier(void) const;
	
	// image path:
	void setImagePath(std::string path);
	std::string imagePath(void) const;
	
	// module inputs:
	void addInputKeyValue(std::string key, std::string v);
	void addInputKeyValue(std::string key, long v);
	void addInputKeyValue(std::string key, float v);
	void addInputKeyValue(std::string key, double v);
	void addInputKeyValue(std::string key, float *v, long n);
	const std::vector<exmParameter*>& inputs(void) const;
	exmParameter* inputParameterForKey(std::string key);
	std::string inputStringForKey(std::string key);
	long inputLongForKey(std::string key);
	double inputRealForKey(std::string key);
	std::vector<float> inputFloatArrayForKey(std::string key);
	std::vector<double> inputDoubleArrayForKey(std::string key);		// just copies the float values!
	std::vector<std::string> inputStringArrayForKey(std::string key);
	
	// module outputs:
	void addOutputKey(std::string);
	
	void setOutputKeyValue(std::string key, std::string v);
	void setOutputKeyValue(std::string key, long v);
	void setOutputKeyValue(std::string key, float v);
	void setOutputKeyValue(std::string key, double v);
	void setOutputKeyValue(std::string key, float *v, long n);
	const std::vector<exmParameter*>& outputs(void) const;
	exmParameter* outputParameterForKey(std::string key);
	
private:
	std::string fuuid;
	std::string imagepath;
	std::string timestamp;
	std::string exmidentifier;
	bool fcompleted;
	std::vector<exmParameter*> finputs;
	std::vector<exmParameter*> foutputs;
	
	exmParameter* parameterForKey(std::vector<exmParameter*>& io, std::string key);
};



class w11Mio {
public:
	
	w11Mio(void);
	~w11Mio();
	
	// file IO:
	void read(std::string path);
	void write(std::string path);
	
	// request IDs:
	exmRequest& addRequestId(std::string puuid);
	std::vector<std::string> requestIdList(void);			/* returns list of all request IDs */
	exmRequest& selectRequestId(std::string puuid);
	
private:
	std::vector<exmRequest*> frequests;
	
};
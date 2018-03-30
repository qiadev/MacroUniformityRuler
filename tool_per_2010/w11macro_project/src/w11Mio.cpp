/*
 *  w11Mio.cpp
 *
 *  Created by Rene Rasmussen on 8/13/09 for INCITS W1.1 Macro-uniformity.
 *
 */

#include "w11Mio.h"
using namespace std;

#include <sstream>

#define EXMVERSION "0.1.0"

w11Mio::w11Mio(void) {
}


w11Mio::~w11Mio()
{
}


void w11Mio::read(string path)
{
	FILE *fin;
	fin= fopen(path.c_str(), "r");
	if (!fin) {
		throw "Failed to open file for reading";
	}
	exmRequest *r= new exmRequest("tempuuid"); 
	r->read(fin);
	frequests.push_back(r);
}

void w11Mio::write(string path)
{
	FILE *fout;
	fout= fopen(path.c_str(), "w");
	if (!fout) {
		throw "Failed to open file for writing";
	}
	vector<exmRequest*>::iterator fit;
	for (fit=frequests.begin(); fit != frequests.end(); ++fit) {
		(*fit)->write(fout);
	}
	fclose(fout);
}

exmRequest& w11Mio::addRequestId(string puuid)
{
	exmRequest *r= new exmRequest(puuid);
	frequests.push_back(r);
	return *r;
}

vector<string> w11Mio::requestIdList(void)
{
	vector<string> ridlist;
	vector<exmRequest*>::iterator fit;
	
	for (fit=frequests.begin(); fit != frequests.end(); ++fit) {
		ridlist.push_back((*fit)->uuid());
	}
	return ridlist;
}

exmRequest& w11Mio::selectRequestId(std::string puuid)
{
	vector<exmRequest*>::iterator fit;
	
	for (fit=frequests.begin(); fit != frequests.end(); ++fit) {
		if ((*fit)->uuid() == puuid) {
			return **fit;
		}
	}
	throw "request not found";
}


//-----------------------------------------------------------------------------------------------------------------------


// external module:

void exmRequest::write(FILE *f)
{
	fprintf(f, "UUID:%s\n",fuuid.c_str());
	fprintf(f, "IMAGE:%s\n",imagepath.c_str());
	fprintf(f, "TIME:%s\n",timestamp.c_str());
	fprintf(f, "EXMID:%s\n",exmidentifier.c_str());
	fprintf(f, "STATUS:%d\n",fcompleted ? 1 : 0);
	// write inputs:
	fprintf(f, "INPUTS:%ld\n", finputs.size());
	vector<exmParameter*>::iterator dit;
	for (dit=finputs.begin(); dit != finputs.end(); ++dit) {
		(*dit)->write(f);
	}
	// write outputs:
	fprintf(f, "OUTPUTS:%ld\n", foutputs.size());
	for (dit=foutputs.begin(); dit != foutputs.end(); ++dit) {
		(*dit)->write(f);
	}
	fprintf(f, "END\n");
}

void exmRequest::read(FILE *f)
{
	char str[1000];
	fscanf(f, "UUID:%s\n", str);
	fuuid= str;
	fscanf(f, "IMAGE:%s\n", str);
	imagepath= str;
	fscanf(f, "TIME:%s\n", str);
	timestamp= str;
	fscanf(f, "EXMID:%s\n", str);
	exmidentifier= str;
	int status= -1;
	fscanf(f, "STATUS:%d\n", &status);
	fcompleted= status == 1;
	//
	int icount= 0;
	fscanf(f, "INPUTS:%d\n", &icount);
	for (long i=0;i<icount;i++) {
		exmParameter *ebd= new exmParameter(f);
		finputs.push_back(ebd);
	}
	
	int ocount= 0;
	fscanf(f, "OUTPUTS:%d\n", &ocount);
	for (long i=0;i<ocount;i++) {
		exmParameter *ebd= new exmParameter(f);
		foutputs.push_back(ebd);
	}
	
}

void exmRequest::setExmIdentifier(string exmid)
{
	exmidentifier= exmid;
}

string exmRequest::exmIdentifier(void) const
{
	return exmidentifier;
}

void exmRequest::setImagePath(string path)
{
	imagepath= path;
}

string exmRequest::imagePath(void) const
{
	return imagepath;
}


// module inputs:
void exmRequest::addInputKeyValue(string key, string v)
{
	exmString* kv= new exmString(v);
	exmParameter *ep= new exmParameter(key, kv);
	finputs.push_back(ep);
}

void exmRequest::addInputKeyValue(string key, long v)
{
	exmLong* kv= new exmLong(v);
	exmParameter *ep= new exmParameter(key, kv);
	finputs.push_back(ep);
}

void exmRequest::addInputKeyValue(string key, float v)
{
	exmFloat* kv= new exmFloat(v);
	exmParameter *ep= new exmParameter(key, kv);
	finputs.push_back(ep);
}

void exmRequest::addInputKeyValue(string key, double v)
{
	exmDouble* kv= new exmDouble(v);
	exmParameter *ep= new exmParameter(key, kv);
	finputs.push_back(ep);
}

void exmRequest::addInputKeyValue(string key, float *v, long n)
{
	exmFloatArray* kv= new exmFloatArray(v, n);
	exmParameter *ep= new exmParameter(key, kv);
	finputs.push_back(ep);
}


const vector<exmParameter*>& exmRequest::inputs(void) const
{
	return finputs;
}

exmParameter* exmRequest::parameterForKey(std::vector<exmParameter*>& io, std::string key)
{
	unsigned long idx= 0;
	
	for (idx=0;idx<io.size(); idx++) {
		if (io[idx]->name() == key) {
			return io[idx];
		}
	}
	ostringstream ost;
	ost << "Parameter '" << key << "' is not known/specified";
	throw ost.str();
}

exmParameter* exmRequest::inputParameterForKey(string key)
{
	return parameterForKey(finputs, key);
}


std::string exmRequest::inputStringForKey(std::string key)
{
	exmParameter* p= inputParameterForKey(key);
	if (p->type() != exm_t_string) {
		throw "Parameter not of type string";
	}
	exmString *ds= (exmString*)p->data();
	return ds->value();
}

long exmRequest::inputLongForKey(std::string key)
{
	exmParameter* p= inputParameterForKey(key);
	if (p->type() != exm_t_long) {
		throw "Parameter not of type long";
	}
	exmLong *ds= (exmLong*)p->data();
	return ds->value();
}

double exmRequest::inputRealForKey(std::string key)
{
	exmParameter* p= inputParameterForKey(key);
	double v= -99;
	switch (p->type()) {
		case exm_t_float: 
		{
			exmFloat *ds= (exmFloat*)p->data();
			v= ds->value();
			break;
		}
		case exm_t_double:
		{
			exmDouble *ds= (exmDouble*)p->data();
			v= ds->value();
			break;
		}
		case exm_t_long:
		{
			exmLong *ds= (exmLong*)p->data();
			v= ds->value();
			break;
		}
		default:
			throw "Parameter not of type real number";
			break;
	}
	return v;
}

vector<float> exmRequest::inputFloatArrayForKey(std::string key)
{
	exmParameter* p= inputParameterForKey(key);
	if (p->type() != exm_t_float_array) {
		throw "Parameter not of type float array";
	}
	exmFloatArray *ds= (exmFloatArray*)p->data();
	return ds->value();
}

vector<double> exmRequest::inputDoubleArrayForKey(std::string key)
{
	exmParameter* p= inputParameterForKey(key);
	if (p->type() != exm_t_float_array) {
		throw "Parameter not of type float array";
	}
	exmFloatArray *ds= (exmFloatArray*)p->data();
	
	vector<double> darray;
	vector<float>::iterator fit;
	for (fit=ds->value().begin(); fit != ds->value().end(); ++fit) {
		darray.push_back(*fit);
	}
	return darray;
}


vector<string> exmRequest::inputStringArrayForKey(std::string key)
{
	exmParameter* p= inputParameterForKey(key);
	if (p->type() != exm_t_string_array) {
		throw "Parameter not of type string array";
	}
	exmStringArray *ds= (exmStringArray*)p->data();
	return ds->value();
}


// module outputs:


exmParameter* exmRequest::outputParameterForKey(string key)
{
	return parameterForKey(foutputs, key);
}

void exmRequest::addOutputKey(string key)
{
	exmParameter* kv= new exmParameter(key, exm_t_unknown);
	foutputs.push_back(kv);
}

void exmRequest::setOutputKeyValue(string key, string v)
{
	exmParameter* p= outputParameterForKey(key);
	exmString* pd= new exmString(v);
	p->replaceData(pd);
	
}

void exmRequest::setOutputKeyValue(string key, long v)
{
	exmParameter* p= outputParameterForKey(key);
	exmLong* pd= new exmLong(v);
	p->replaceData(pd);
}

void exmRequest::setOutputKeyValue(string key, float v)
{
	exmParameter* p= outputParameterForKey(key);
	exmFloat* pd= new exmFloat(v);
	p->replaceData(pd);
}

void exmRequest::setOutputKeyValue(string key, double v)
{
	exmParameter* p= outputParameterForKey(key);
	exmDouble* pd= new exmDouble(v);
	p->replaceData(pd);
}

void exmRequest::setOutputKeyValue(string key, float *v, long n)
{
	exmParameter* p= outputParameterForKey(key);
	exmFloatArray* pd= new exmFloatArray(v, n);
	p->replaceData(pd);
}


const vector<exmParameter*>& exmRequest::outputs(void) const
{
	return foutputs;
}
//-----------------------------------------------------------------------------------------------------------------------


exmString::exmString(std::string _value)
{
	fValue= _value;
}


exmLong::exmLong(long _value)
{
	fValue= _value;
}

exmFloat::exmFloat(float _value)
{
	fValue= _value;
}

exmDouble::exmDouble(double _value)
{
	fValue= _value;
}

exmFloatArray::exmFloatArray(float* _value, long n)
{
	fValue.reserve(n);
	for (long i=0;i<n;i++) {
		fValue.push_back(_value[i]);
	}
}

exmStringArray::exmStringArray(std::vector<std::string> _value)
{
	fValue.reserve(_value.size());
	for (unsigned long i=0;i<_value.size();i++) {
		fValue.push_back(_value[i]);
	}
}

exmParameter::exmParameter(std::string _name, TExmDataType _type) {
	init_exmParameter(_name, _type);
};

exmParameter::exmParameter(std::string _name, exmData *_ed)
{
	init_exmParameter(_name, _ed->type());
	fdata= _ed;
}

exmParameter::exmParameter(FILE *f)
{
	TExmDataType itype= exm_t_unknown;
	
	//int iitype=0;
	//fscanf(f, "%d::", &iitype);
	//itype= (TExmDataType) iitype;
	char ch1='\0', ch2='\0';
	if (fscanf(f, "%c%c::", &ch1, &ch2) != 2) {
		throw string("Error reading parameter type");
	}
	itype= typeFromCharCode(ch1, ch2);

	char ch= '\0';
	ostringstream ost;
	bool finished= false;
	do {
		ch= fgetc(f);
		if (ch == ':') {
			ch2= fgetc(f);
			if (ch2 == ':') {
				finished= true;
			} else {
				ost << ch << ch2;
			}
		} else {
			ost << ch;
		}
	} while (!finished);
	
	init_exmParameter(ost.str().c_str(), itype);
	if (itype != exm_t_unknown) {
		fdata->read(f);
	} else {
		char line[1000];
		fgets(line, 999, f);
	}
}

TExmDataType exmParameter::typeFromCharCode(const char ch1,const char ch2) const
{
	TExmDataType ptype= exm_t_invalid;
	if (ch1 == '0' && ch2 == '0') {
		ptype= exm_t_unknown;
	} else if (ch1 == 'S' && ch2 == 'S') {
		ptype= exm_t_string;
	} else if (ch1 == 'L' && ch2 == 'S') {
		ptype= exm_t_long;
	} else if (ch1 == 'F' && ch2 == 'S') {
		ptype= exm_t_float;
	} else if (ch1 == 'D' && ch2 == 'S') {
		ptype= exm_t_double;
	} else if (ch1 == 'S' && ch2 == 'A') {
		ptype= exm_t_string_array;
	} else if (ch1 == 'F' && ch2 == 'A') {
		ptype= exm_t_float_array;
	} else {
		ptype= exm_t_invalid;
	}
	return ptype;
}

string exmParameter::charCodeFromType(const TExmDataType ptype) const 
{
	string ps;
	switch (ptype) {
		case exm_t_unknown:
			ps= "00";
			break;
		case exm_t_string:
			ps= "SS";
			break;
		case exm_t_long:
			ps= "LS";
			break;
		case exm_t_float:
			ps= "FS";
			break;
		case exm_t_double:
			ps= "DS";
			break;
		case exm_t_string_array:
			ps= "SA";
			break;
		case exm_t_float_array:
			ps= "FA";
			break;
		default:
			ps= "99";
			break;
	}
	return ps;
}

void exmParameter::init_exmParameter(std::string _name, TExmDataType _type)
{
	fName= _name;
	dtype= _type;
	switch (dtype) {
		case exm_t_unknown:
			break;
		case exm_t_string:
			fdata= new exmString();
			break;
		case exm_t_long:
			fdata= new exmLong();
			break;
		case exm_t_float:
			fdata= new exmFloat();
			break;
		case exm_t_double:
			fdata= new exmDouble();
			break;
		case exm_t_float_array:
			fdata= new exmFloatArray();
			break;
		case exm_t_string_array:
			fdata= new exmStringArray();
			break;
		case exm_t_invalid:
			throw "init_exmParameter - invalid type";
			break;
		default:
			throw "init_exmParameter - type not implemented";
	}
}

void exmParameter::replaceData(exmData *_ed)
{
	if (fdata) {
		delete fdata;
	}
	dtype= _ed->type();
	fdata= _ed;
}


void exmParameter::write(FILE *f) const {
	fprintf(f, "%2s::%s::", charCodeFromType(dtype).c_str(), fName.c_str());
	if (dtype == exm_t_unknown) {
		fprintf(f, "<<unknown_type>>\n");
	} else {
		fdata->write(f);
	}
}

void exmString::read(FILE *f)
{
	char line[1000];
	fgets(line, 1000,f);		// line contains the newline char
	line[strlen(line)-1]= '\0';
	fValue= line;
}

void exmString::write(FILE *f) const {
	fprintf(f, "%s\n", fValue.c_str());
}

void exmLong::read(FILE *f)
{
	char line[1000];
	fgets(line, 1000,f);
	sscanf(line, "%ld", &fValue);
}

void exmLong::write(FILE *f) const
{
	fprintf(f, "%ld\n", fValue);
}

void exmFloat::read(FILE *f)
{
	char line[1000];
	fgets(line, 1000,f);
	sscanf(line, "%f", &fValue);
}

void exmFloat::write(FILE *f) const
{
	fprintf(f, "%f\n", fValue);	
}

void exmDouble::read(FILE *f)
{
	char line[1000];
	fgets(line, 1000,f);
	sscanf(line, "%lf", &fValue);
}

void exmDouble::write(FILE *f) const
{
	fprintf(f, "%lf\n", fValue);	
}

void exmFloatArray::read(FILE *f)
{
	long n= 0;
	float v;
	fscanf(f, "%ld::", &n);
	fValue.reserve(n);
	for (long i=0;i<n;i++) {
		fscanf(f, "%f", &v);
		fValue.push_back(v);
	}
	//
	char line[1000];
	fgets(line, 1000,f);	// discard the rest "EOA\n"
}

void exmFloatArray::write(FILE *f) const
{
	fprintf(f, "%ld::", fValue.size());
	for (unsigned long i=0;i<fValue.size(); i++) {
		fprintf(f, "%8e ", fValue[i]);
	}
	fprintf(f, " EOA\n");
}


void exmStringArray::read(FILE *f)
{
	char line[1000];
	long n= 0;
	fscanf(f, "%ld::", &n);
	fgets(line, 1000,f);	// discard the rest of the line
	fValue.reserve(n);
	for (long i=0;i<n;i++) {
		fgets(line, 1000, f);		// line contains the newline char
		line[strlen(line)-1]= '\0';
		fValue.push_back(line);
	}
}

void exmStringArray::write(FILE *f) const
{
	fprintf(f, "%ld::", fValue.size());
	for (unsigned long i=0;i<fValue.size(); i++) {
		fprintf(f, "%s\n", fValue[i].c_str());
	}
}











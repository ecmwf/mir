/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Exception.h"

#include <sstream>

Exception::Exception() 
{
}

Exception::Exception(const string& w):
	what_(w)
{
}

Exception::~Exception() throw()
{
}


void Exception::print(ostream& out) const
{
	out << "Exception: " << what_ << endl;
}

AssertionFailed::AssertionFailed(const string& w) :
	Exception("Assertion failed: " + w)
{
}

AssertionFailed::AssertionFailed(const char* msg, int line, const char* file, const char* proc) :
	Exception("Assertion failed: ")
{
	stringstream s;

	s << "Assertion failed: " << msg << " in " << proc
	<< ", line " << line << " of " << file ;

	reason(s.str());

	cout << *this << endl;
}

BadParameter::BadParameter(const string& w):
    Exception(("Bad parameter: ") + w)
{
}

Failed::Failed(const string& w):
    Exception(w + (" FAILED!"))
{
}

WrongGaussianNumber::WrongGaussianNumber(int n):
    Exception(("Wrong Gaussian: "))
{
	stringstream s;

	s << "Gaussian Number " << n << " is Not Valid ";

	reason(s.str());
}

WrongIncrements::WrongIncrements(double we, double ns):
    Exception(("Wrong Increments: "))
{
	stringstream s;

	s << "Wrong Increments West-East " << we << " North-South " << ns;

	reason(s.str());
}

FactorHandled::FactorHandled(int la, int factor, int n ):
    Exception(("Factor Handled: "))
{
	stringstream s;

	s << "Factor only handled if la * factor = numberOfPoints  la: " << la << "* factor: " << factor << " != numberOfPoints: " << n ;

	reason(s.str());
}

WrongArea::WrongArea(const string& side, double b):
    Exception(("Wrong Boundary: "))
{
	stringstream s;

	s << "Wrong " << side << " Boundary: " << b;

	reason(s.str());
}

WrongValue::WrongValue(const string& st, double a):
    Exception(("Wrong Value: "))
{
	stringstream s;

	s << st << ": " << a;

	reason(s.str());
}

NotImplemented::NotImplemented(const string& in, const string& out, const string& info/*=""*/) :
	Exception("Not Implemented Transformation: ")
{
	stringstream s;

	s << "Transformation  from " << in << " To " << out << " is Not Implemented " << info;

	reason(s.str());
}

NotImplementedFeature::NotImplementedFeature(const string& r) :
	Exception("Not Implemented Feature for : " + r)
{
}

WrongFunctionCall::WrongFunctionCall(const string& r) :
	Exception("Wrong Function Call : " + r)
{
}

UserError::UserError(const string& x, const string& y):
	Exception("UserError: " + x + y)
{
//	stringstream s;
//	s << "UserError: " << x << " To " << y ;
//	reason(s.str());
}

OutOfArea::OutOfArea(double lat, double lon):
	Exception("OutOfArea: ")
{
	stringstream s;
    s << "Pair of Latitude: " << lat
      << ", longitude " << lon << " is out of Area ";
    reason(s.str());
}

OutOfRange::OutOfRange(unsigned long long index, unsigned long long max):
	Exception("OutOfRange: ")
{
	stringstream s;
    s << "Out of range accessing element " << index
      << ", but maximum is " << max - 1 ;
    reason(s.str());
}

CantOpenFile::CantOpenFile(const string& file):
	Exception("Can't Open File: " + file)
{
}

WriteError::WriteError(const string& file):
    Exception("Write error on " + file)
{
}

ReadError::ReadError(const string& file):
    Exception("Read error on " + file)
{
}

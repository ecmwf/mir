/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Exceptions_H
#define Exceptions_H

#include<stdexcept>

#ifndef machine_H
#include "machine.h"
#endif

// General purpose exception
// Misc. errors

class Exception : public exception {
//class Exception {

  public:
    Exception(const string& w);
    virtual ~Exception() throw();

    virtual const char* what() const throw() {
        return what_.c_str();
    }

  protected:
    void reason(const string& w) {
        what_ = w;
    }
    void print(ostream&) const;
    Exception();

  private:
    string what_;

// -- Friends
    friend ostream& operator<<(ostream& s,const Exception& e) {
        e.print(s);
        return s;
    }
};


class AssertionFailed : public Exception {
  public:
    AssertionFailed(const string& w);
    AssertionFailed(const char* msg,int line, const char* file, const char* proc);
};

class BadParameter : public Exception {
  public:
    BadParameter(const string& w);
};

class Failed : public Exception {
  public:
    Failed(const string& w);
};

class WrongGaussianNumber : public Exception {
  public:
    WrongGaussianNumber(int n);
};

class WrongIncrements : public Exception {
  public:
    WrongIncrements(double we, double ns);
};

class FactorHandled: public Exception {
  public:
    FactorHandled(int la, int factor, int n);
};

class WrongArea : public Exception {
  public:
    WrongArea(const string& side, double b);
};

class WrongValue : public Exception {
  public:
    WrongValue(const string& side, double b);
};

class NotImplemented : public Exception {
  public:
    NotImplemented(const string& in, const string& out, const string& info="");
};

class NotImplementedFeature : public Exception {
  public:
    NotImplementedFeature(const string&);
};

class WrongFunctionCall : public Exception {
  public:
    WrongFunctionCall(const string&);
};

class UserError : public Exception {
  public:
    UserError(const string& x, const string& y="\n");
};

class OutOfArea : public Exception {
  public:
    OutOfArea(double lat, double lon);
};

class OutOfRange : public Exception {
  public:
    OutOfRange(unsigned long long index, unsigned long long max);
};

class CantOpenFile : public Exception {
  public:
    CantOpenFile(const string&);
};

class WriteError : public Exception {
  public:
    WriteError(const string&);
};

class ReadError : public Exception {
  public:
    ReadError(const string&);
};

// =======================================

inline void Assert(int code,const char *msg,int line,const char *file,
                   const char *proc) {
    if(code != 0)
        throw AssertionFailed(msg,line,file,proc);
}


#define ASSERT(a)  Assert(!(a),#a,__LINE__,__FILE__,__FUNCTION__)

#endif

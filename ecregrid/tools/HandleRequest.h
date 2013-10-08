/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef HandleRequest_H
#define HandleRequest_H


// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif


class HandleRequest {
public:

// -- Exceptions
	// None

// -- Contructors
	HandleRequest();

// -- Destructor

	~HandleRequest(); // Change to virtual if base class

// -- Operators
	// None

// -- Methods

    void  inFile(const string& in) { inFile_ = in; }
    void  outFile(const string& out){ outFile_ = out; }
    void  outFileType(const string& out){ outFileType_ = out; }

	void  usage() const;
	bool isVortDivConversion(int param) const { return (param == 155 || param == 138) && output_->vdConversion();}

	int  request(int argc, char* argv[]);
	void  processing() const ;
	bool isWindPair(int u, int v) const;
	bool pairOrder(int u, int v) const;

protected:
	void print(ostream&) const;

private:

// No copy allowed

	HandleRequest(const HandleRequest&);
	HandleRequest& operator=(const HandleRequest&);

    void showVersionInfo() const;
// -- Members
	auto_ptr<FieldDescription>  output_;

	string inFile_;
	string outFile_;
	string outFileType_;

// -- Friends

	friend ostream& operator<<(ostream& s,const HandleRequest& p)
	{ p.print(s); return s; }

};

#endif

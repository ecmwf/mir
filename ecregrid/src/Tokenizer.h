/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Tokenizer_H
#define Tokenizer_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#include <fstream>
#include <vector>

using std::vector;
using std::fstream;


class Tokenizer {
public:

// -- Contructors

//    Tokenizer(const string& separator, const string& comment="#");
    Tokenizer(const string& separator);

// -- Destructor

	~Tokenizer(); // Change to virtual if base class

// -- Methods
	
	void operator()(const string&, vector<string>&);
	void operator()(istream&,vector<string>&);
	void operator()(fstream&,vector<string>&);

private:

// No copy allowed

	Tokenizer(const Tokenizer&);
	Tokenizer& operator=(const Tokenizer&);

// -- Members

	 string separators_;     // To make searching faster
	 string comment_;

// -- Methods

	void print(ostream&) const;

	friend ostream& operator<<(ostream& s,const Tokenizer& p)
		{ p.print(s); return s; }

};

#endif

/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef   Tokenizer_H
#include "Tokenizer.h"
#endif

/*
Tokenizer::Tokenizer(const string& separators, const string& comment):
	separators_(separators), comment_(comment)
{
}
*/

Tokenizer::Tokenizer(const string& separators):
	separators_(separators), comment_("#")
{
}

Tokenizer::~Tokenizer()
{
}


void Tokenizer::operator()(const string& str, vector<string>& tokens)
{
   // Skip separators at beginning.
    string::size_type lastPos = str.find_first_not_of(separators_, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(separators_, lastPos);
	// Find comment
    string::size_type cpos     = str.find_first_of(comment_, 0);

//    while (string::npos != pos || string::npos != lastPos)
	while ((string::npos != pos || string::npos != lastPos) && string::npos == cpos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip separators.  Note the "not_of"
        lastPos = str.find_first_not_of(separators_, pos);
        // Find next "non-separator"
        pos = str.find_first_of(separators_, lastPos);
	    // Exit if Find comment
		cpos     = str.find_first_of(comment_, 0);
    }


}

void Tokenizer::operator()(istream& in, vector<string>& v)
{
    string raw;
    char c;

    while(in.get(c) && c != EOF && c != '\n')
        raw += c;

    operator()(raw,v);
}

void Tokenizer::operator()(fstream& in, vector<string>& v)
{
    string raw;
    char c;

    while(in.get(c) && c != EOF && c != '\n')
        raw += c;

    operator()(raw,v);
}



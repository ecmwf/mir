/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Bitmap.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Tokenizer_H
#include <eckit/parser/Tokenizer.h>
#endif

#ifndef GridField_H
#include "GridField.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

Bitmap::Bitmap(const string& fileName, double missingValue) :
    Extraction(), fileName_(fileName), missingValue_(missingValue) {
}

Bitmap::~Bitmap() {
}

void Bitmap::setBitmap(vector<bool>& bitmap, long offset, int firstColumn, int lastColumn, bool value) const {
    for(int columnNumber=firstColumn; columnNumber<=lastColumn; columnNumber++) {
        long next = offset + columnNumber;
        bitmap[next] = value;
    }
}

void Bitmap::extract(const Grid& output, vector<double>& values) const {
    eckit::Tokenizer comma(",");
    eckit::Tokenizer slash("/");
    eckit::Tokenizer dash("-");
    eckit::Tokenizer colon(":");
    eckit::Tokenizer equal("=");
    string buffer;

    bool defaultValue = false;
    bool        value = true;

    int weNumber, nsNumber;

    const size_t valuesSize = values.size();
    vector<bool> bitmap(valuesSize, defaultValue);

    ifstream is;
    is.open(fileName_.c_str());
    if (!is)
        throw CantOpenFile("Bitmap::extract " + fileName_);

    while( getline(is,buffer) ) {
//		cout << buffer << endl;
        vector<string> v;
        comma(buffer,v);

        int size = v.size();

        if (size >= 1) {
            for(int i = 0; i < size ; i++) {
                vector<string> n;
                equal(v[i],n);
                if(DEBUG)
                    cout << " Bitmap::extract => comma  " << v[i] << endl;

                if(n[0] == "SPEC") {
                    if(DEBUG)
                        cout << " Bitmap::extract => SPEC is present" << endl;
                } else if(n[0] == "SIZE") {
                    vector<string> s;
                    colon(n[1],s);
                    weNumber = atoi(s[0].c_str());
                    nsNumber = atoi(s[1].c_str());
                } else if(n[0] == "VALUES") {
                    if(n[1] == "ON")
                        defaultValue = true;
                    value = false;
                    for (int j = 0 ; j < size ; j++)
                        bitmap[j] = defaultValue;
                } else if(n[0] == "POINTS") {
                    vector<string> s;
                    colon(n[1],s);
                    int ssize = s.size();
                    if(DEBUG) {
                        for(int j = 0; j < ssize ; j++)
                            cout << " Bitmap::extract => colon POINTS  " << s[i] << endl;
                    }
                } else {
                    vector<string> s;
                    colon(v[i],s);
                    int ssize = s.size();
                    if(DEBUG) {
                        for(int j = 0; j < ssize ; j++)
                            cout << " Bitmap::extract => colon  " << s[i] << endl;
                    }
                }
            }
        }

    }
    is.close();

    setBitmap(bitmap,3,7,14,value);

    for (unsigned int j = 0 ; j < valuesSize ; j++)
        if(bitmap[j] != defaultValue)
            values[j] = missingValue_;

}

void Bitmap::print(ostream& out) const {
    out << "Bitmap file name " << fileName_ ;
}


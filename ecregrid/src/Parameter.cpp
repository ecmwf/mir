/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Parameter.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Tokenizer_H
#include <eckit/utils/Tokenizer.h>
#endif

#include "DefinitionsCache.h"

#include <sstream>


Parameter::Parameter() :
    number_(0),
    table_(128),
    lsm_(false),
    conservation_(false),
    nearest_(false),
    wind_(false),
    vertical_(1) {
}

Parameter::Parameter(int param, int table, const string& levType) :
    number_(param),
    table_(table),
    lsm_(false),
    conservation_(false),
    nearest_(false),
    wind_(false),
    vertical_(1) {
    if (levType == "sfc")
        lsm_ = true;
    if (param)
        peculiarParameters();
}

Parameter::Parameter(const Parameter& other) :
    number_(other.number_),
    table_(other.table_),
    lsm_(other.lsm_),
    conservation_(other.conservation_),
    nearest_(other.nearest_),
    wind_(other.wind_),
    vertical_(other.vertical_) {
}

Parameter::~Parameter() {
}

Parameter& Parameter::operator=(const Parameter& id) {
    if (this == &id)
        return *this;

    number_       = id.number_;
    table_        = id.table_;
    lsm_          = id.lsm_;
    conservation_ = id.conservation_;
    nearest_      = id.nearest_;
    wind_         = id.wind_;
    vertical_     = id.vertical_;

    return *this;
}

int  Parameter::marsParam() const {
    if(number_ == 0) {
        cout << "WARNING - Parameter::marsParam paramId is 0 reset to 1" << endl;
        return 1;
    }

    if(table_ == 128)
        return number_;

    return table_ * 1000 + number_;
}

void Parameter::peculiarParameters() {
    eckit::Tokenizer tokens(" ");

    stringstream s;
    s << getShareDir() << "/definitions/parameters" ;

    string fileName = s.str();
    ref_counted_ptr< const vector<string> > paramData = DefinitionsCache::get(fileName);

    vector<string>::const_iterator it = paramData->begin();

    while (it != paramData->end() ) {
        vector<string> v;
        tokens(*it, v);

        if (v.size() > 1) {
            if ( atoi(v[0].c_str()) == number_ && atoi(v[1].c_str()) == table_ ) {
                lsm_          = atoi(v[2].c_str());
                wind_         = atoi(v[3].c_str());
                conservation_ = atoi(v[4].c_str());
                nearest_      = atoi(v[5].c_str());
                vertical_     = atoi(v[6].c_str());
            }
        }
        it++;
    }
    if(DEBUG) {
        cout << "Parameter::peculiarParameters path"  << s.str() << endl;
        cout << "Number: "  << number_ << " Table: " << table_ << " Lsm: " << lsm_ << " Wind: " << wind_ << " Vertical Type " << vertical_ << endl;
    }
}


void Parameter::print(ostream& out) const {
    out << "Parameter{ number=[" << number_ << "], table=[" << table_ << "], lsm=[" << lsm_ << "], wind=[" << wind_ << "], conservation=["<< conservation_ << "], nearest=["<< nearest_ << "], vertical=["<< vertical_ << "] }";
}

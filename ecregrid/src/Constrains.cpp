/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "Constrains.h"

#ifndef Tokenizer_H
#include <eckit/utils/Tokenizer.h>
#endif

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef FieldDescription_H
#include "FieldDescription.h"
#endif

#include "DefinitionsCache.h"

#include <sstream>


Constrains::Constrains(const string& in, const string& out):
    in_(in), out_(out) {
}

Constrains::~Constrains() {
}

void Constrains::isPossible(FieldDescription* methods, bool global_output) const {
    eckit::Tokenizer tokens("  ");

    stringstream s;
    s << getShareDir() << "/definitions/constrains" ;
    string fileName = s.str();

    ref_counted_ptr< const vector<string> > constrainsData= DefinitionsCache::get(fileName);

    vector<string>::const_iterator it = constrainsData->begin();

    bool found = false;
    std::string exception_info;

    while (it != constrainsData->end() ) {
        vector<string> v;
        tokens(*it, v);
        if(in_ == v[0] && out_ == v[1]) {
            if(v[2] == "false")
                break;  // to throw below

            if (v[2] == "global" && !global_output) {
                exception_info = "for non-global output grids";
                break;
            }

#if ECREGRID_EMOS_SIMULATION
            int size = v.size();
            if(size > 3)
                methods->interpolationMethod(v[3]);
            if(size > 4)
                methods->lsmMethod(v[4]);
#endif
            found = true;
            break;
        }

        it++;
    }

    if (!found)
        throw NotImplemented(in_, out_, exception_info);
}

void Constrains::print(ostream&) const {
    cout << "Constrains::print Tranformation from " << in_ << " To " << out_ << endl;
}

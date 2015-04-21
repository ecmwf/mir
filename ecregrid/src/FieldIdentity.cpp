/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FieldIdentity.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Tokenizer_H
#include <eckit/utils/Tokenizer.h>
#endif

#include "DefinitionsCache.h"

FieldIdentity::FieldIdentity(const string &name):
    composedName_(name), gridOrSpectral_(true), gridType_("none"), reduced_(false), stretched_(false), rotated_(false) {
    setFromTypeOfGrid(name);
}

FieldIdentity::FieldIdentity(bool gridOrSpectral, const string &gridType, bool reduced, bool stretched, bool rotated):
    composedName_("none"), gridOrSpectral_(gridOrSpectral), gridType_(gridType), reduced_(reduced), stretched_(stretched), rotated_(rotated) {
}

FieldIdentity::FieldIdentity():
    composedName_("none"), gridOrSpectral_(true), gridType_("none"), reduced_(false), stretched_(false), rotated_(false) {
}

FieldIdentity::FieldIdentity(const FieldIdentity &other):
    composedName_(other.composedName_), gridOrSpectral_(other.gridOrSpectral_), gridType_(other.gridType_), reduced_(other.reduced_), stretched_(other.stretched_), rotated_(other.rotated_) {
}

FieldIdentity::~FieldIdentity() {
}

void FieldIdentity::setFromTypeOfGrid(const string &name) {
    const string fileName = getShareDir() + "/definitions/identity_table" ;

    eckit::Tokenizer tokens(" ");
    bool found = false;

    ref_counted_ptr< const vector<string> > identityTable = DefinitionsCache::get(fileName);

    vector<string>::const_iterator it = identityTable->begin();

    while (it != identityTable->end() && !found) {
        vector<string> v;
        tokens(*it, v);

        if (v.size() > 1) {
            if (name == v[0]) {
                composedName_   = v[0];
                gridOrSpectral_ = atoi(v[1].c_str());
                gridType_       = v[2];
                reduced_        = atoi(v[3].c_str());
                stretched_      = atoi(v[4].c_str());
                rotated_        = atoi(v[5].c_str());
                found = true;
            }
        }

        it++;
    }

    if (!found)
        throw UserError("FieldIdentity::setFromTypeOfGrid -- There is Not Field with this Composed Name -> " + name);
}

void FieldIdentity::isAvailable() {
    eckit::Tokenizer tokens(" ");
    bool found = false;

    const string fileName = getShareDir() + "/definitions/identity_table";
    ref_counted_ptr< const vector<string> > identityTable = DefinitionsCache::get(fileName);

    vector<string>::const_iterator it = identityTable->begin();

    while (it != identityTable->end() && !found) {
        vector<string> v;
        tokens(*it, v);

        if (v.size() > 1) {
            if (composedName_ != "none") {
                if (composedName_ == v[0]) {
                    found = true;
                }
            } else if ( gridOrSpectral_ == atoi(v[1].c_str()) && gridType_ == v[2] && reduced_ == atoi(v[3].c_str()) && stretched_ == atoi(v[4].c_str()) && rotated_ == atoi(v[5].c_str())) {
                composedName_ = v[0];
                found = true;
            }
        }

        it++;
    }

    if (DEBUG)
        cout << "FieldIdentity::isAvailable Type of Field is: " << composedName_ << endl;

    if (!found)
        throw UserError("FieldIdentity::isAvailable -- Unavailable grid type: ", gridType_ );
}

FieldIdentity &FieldIdentity::operator=(const FieldIdentity &id) {
    if (this == &id)
        return *this;

    composedName_   = id.composedName_;
    gridOrSpectral_ = id.gridOrSpectral_;
    gridType_       = id.gridType_;
    reduced_        = id.reduced_;
    stretched_      = id.stretched_;
    rotated_        = id.rotated_;

    return *this;
}

bool FieldIdentity::operator==(const FieldIdentity &id) const {
    return composedName_ == id.composedName_ && gridOrSpectral_ == id.gridOrSpectral_ && gridType_ == id.gridType_ && reduced_ == id.reduced_ && stretched_ == id.stretched_ && rotated_ == id.rotated_;
}

bool FieldIdentity::operator|(const FieldIdentity &id) const {
    return gridOrSpectral_ == id.gridOrSpectral_ && gridType_ == id.gridType_ && reduced_ == id.reduced_ && stretched_ == id.stretched_ && rotated_ == id.rotated_;
}

void FieldIdentity::print(ostream &out) const {
    out << " Field Identity{  Composed Name =[" << composedName_ << "], gridOrSpectral=[" << gridOrSpectral_ << "] gridType=[" << gridType_ << "] Reduced=[" << reduced_ << "] Stretched [" << stretched_ << "] Rotated=[" << rotated_ << "] }";
}

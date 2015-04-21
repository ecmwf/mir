/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "FieldPoint.h"

FieldPoint::FieldPoint(double lat, double lon, int i, int j, long k, double value):
    Point(lat, lon, i, j, k), value_(value) {
}

FieldPoint::FieldPoint(double lat, double lon, int i, int j, double value):
    Point(lat, lon, i, j), value_(value) {
}

FieldPoint::FieldPoint(const FieldPoint &p):
    Point(p.latitude_, p.longitude_, p.iIndex_, p.jIndex_, p.k1dIndex_), value_(p.value_) {
}

FieldPoint::FieldPoint(const Point &p, double value):
    Point(p), value_(value) {
}

FieldPoint::~FieldPoint() {
}

FieldPoint &FieldPoint::operator=(const FieldPoint &p) {
    if (this == &p)
        return *this;

    latitude_    = p.latitude_;
    longitude_   = p.longitude_;

    iIndex_  = p.iIndex_;
    jIndex_  = p.jIndex_;
    k1dIndex_ = p.k1dIndex_;
    value_   = p.value_;

    return *this;
}

bool FieldPoint::operator==(const FieldPoint &p) const {
    return latitude_ == p.latitude_ && longitude_ == p.longitude_ && iIndex_ == p.iIndex_ && jIndex_ == p.jIndex_ && k1dIndex_ == p.k1dIndex_ && value_ == p.value_;
}

void FieldPoint::print(ostream &out) const {
    out << "FieldPoint{ " ;
    Point::print(out);
    out << ", data=[" << value_ << "]}";
}

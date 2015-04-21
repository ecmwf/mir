/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef FieldPoint_H
#define FieldPoint_H

#include "Point.h"

class FieldPoint : public Point {
  public:

    FieldPoint(double latitude, double longitude, int i, int j, long k, double value);
    FieldPoint(double latitude, double longitude, int i, int j, double value);
    FieldPoint(const Point &p, double value);
    FieldPoint(const FieldPoint &);

    ~FieldPoint();

    FieldPoint &operator=(const FieldPoint &);
    bool operator ==(const FieldPoint &other) const;

    double value() const {
        return value_;
    }
    bool operator <(const FieldPoint &other) const {
        return latitude_ == other.latitude_ ? longitude_ < other.longitude_ : latitude_ > other.latitude_;
    }


  private:

    // No copy allowed
    //  ssp why has to be commented
    //	FieldPoint(const FieldPoint&);
    //	FieldPoint& operator=(const FieldPoint&);

    void print(ostream &) const;

    double value_;

    // -- Friends

    friend ostream &operator<<(ostream &s, const FieldPoint &p) {
        p.print(s);
        return s;
    }
};


#endif


#ifndef Rotation_H
#define Rotation_H
/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

// Headers
#include "machine.h"

// Forward declarations
class Point;

class Rotation {
  public:

// -- Exceptions
    // None

// -- Contructors

    Rotation(double lat = 0, double lon = 0);

// -- Destructor

    ~Rotation(); // Change to virtual if base class

// -- Operators
    bool operator==(const Rotation&) const;

// -- Methods
    double latPole() const {
        return latPole_;
    }
    double lonPole() const {
        return lonPole_;
    }
    bool   isRotated() const {
        return latPole_ > 0 || lonPole_ > 0 ;
    }

    int    match(int truncation)         const;

    Point  rotate(const Point& p)        const;
    Point  unRotate(const Point& p)      const;
    void   angularChange(const vector<Point>& gridPoints, vector<double>& angularChange) const;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members
    // None

// -- Methods

    void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    Rotation(const Rotation&);
    Rotation& operator=(const Rotation&);

// -- Members
    double latPole_;
    double lonPole_;

// -- Methods
    Point rotateLatitude(const Point& p, const string& what)  const;
    Point rotateLongitude(const Point& p, const string& what) const;
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend ostream& operator<<(ostream& s,const Rotation& p) {
        p.print(s);
        return s;
    }
};

#endif

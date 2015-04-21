/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomialsSharedMemory_H
#define LegendrePolynomialsSharedMemory_H


#ifndef LegendrePolynomialsRead_H
#include "LegendrePolynomialsRead.h"
#endif

#include <vector>
// Forward declarations


class LegendrePolynomialsSharedMemory : public LegendrePolynomialsRead {
  public:

// -- Exceptions
    // None

// -- Contructors

    LegendrePolynomialsSharedMemory(int truncation, const Grid& grid);

// -- Destructor

    ~LegendrePolynomialsSharedMemory(); // Change to virtual if base class

  protected:

// Overriden methods
    void print(ostream&) const;

  private:

// No copy allowed

    LegendrePolynomialsSharedMemory(const LegendrePolynomialsSharedMemory&);
    LegendrePolynomialsSharedMemory& operator=(const LegendrePolynomialsSharedMemory&);

// -- Members
    int     sem_;
    void*   ptr_;

    std::vector<void*> segments_;
    size_t  length_;
    double* next_;

// -- Overridden methods
    const double* getOneLatitude(double lat, int rowOffset) const;

    const double* getPolynoms() const {
        return next_;
    }

// -- Class members
    void detachSegments();

// -- Friends
    friend ostream& operator<<(ostream& s,const LegendrePolynomialsSharedMemory& p) {
        p.print(s);
        return s;
    }

};

#endif

/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef LegendrePolynomialsRead_H
#define LegendrePolynomialsRead_H

#ifndef LegendrePolynomials_H
#include "LegendrePolynomials.h"
#endif

class Grid;

class LegendrePolynomialsRead : public LegendrePolynomials {
  public:

// -- Contructors

    LegendrePolynomialsRead(int truncation, const Grid& grid);

// -- Destructor

    virtual ~LegendrePolynomialsRead(); // Change to virtual if base class

// -- Methods
    void createWholeGlobeAndWriteToFile(const Grid& grid) const;
    bool checkFileSize(FILE* f, int globalLatNumber );
    virtual void checkAndPossiblyCreate(const Grid& grid);

  protected:

// -- Members
    string  gridInfo_;
    long    latLength_;
    string  path_;
    long    latSize_ ;
    mutable int fd_;

// -- Overridden methods
    void print(ostream&) const; // Change to virtual if base class
    string constructCoefficientsFilename() const;
    string constructFilename(const string& stub) const;
    bool openCoefficientsFile(int flags=0) const;
    void closeFile() const;

  private:

// No copy allowed

    LegendrePolynomialsRead(const LegendrePolynomialsRead&);
    LegendrePolynomialsRead& operator=(const LegendrePolynomialsRead&);
};

#endif

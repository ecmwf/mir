/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Bitmap_H
#define Bitmap_H


// Headers
#ifndef   Extraction_H
#include "Extraction.h"
#endif

// Forward declarations
class GridField;

class Bitmap : public Extraction {
  public:

// -- Exceptions
    // None

// -- Contructors

    Bitmap(const string& number, double missingValue);

// -- Destructor

    ~Bitmap();


// -- Methods
    void extract(const Grid& output, vector<double>& values) const;
    void setBitmap(vector<bool>& bm, long offset, int firstColumn, int lastColumn, bool value) const;

  protected:
    void print(ostream&) const;

  private:

// No copy allowed

    Bitmap(const Bitmap&);
    Bitmap& operator=(const Bitmap&);

    string fileName_;
    double missingValue_;

// -- Friends

    friend ostream& operator<<(ostream& s,const Bitmap& p) {
        p.print(s);
        return s;
    }

};

#endif

/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Frame_H
#define Frame_H

// Headers
#ifndef   Extraction_H
#include "Extraction.h"
#endif

// Forward declarations
class GridField;
class Grid;

class Frame : public Extraction {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Frame(int number, double missingValue);

    // -- Destructor

    ~Frame();


    // -- Methods
    void extract(const Grid &output, vector<double> &values) const;

  protected:
    void print(ostream &) const;

  private:
    // No copy allowed

    Frame(const Frame &);
    Frame &operator=(const Frame &);

    int numberOfPointsAcrossTheFrame_;
    double missingValue_;

    // -- Friends

    friend ostream &operator<<(ostream &s, const Frame &p) {
        p.print(s);
        return s;
    }

};

#endif

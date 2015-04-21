/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Input_H
#define Input_H

#ifndef   machine_H
#include "machine.h"
#endif

#include <vector>

// Forward declarations
class Field;
class FieldDescription;
class Point;
class Grid;

class Input {
  public:

// -- Contructors

    Input();
    Input(const string& name);

// -- Destructor

    virtual ~Input(); // Change to virtual if base class

// -- Operators
    // None

// -- Methods
    const string& fileName() const {
        return fileName_;
    }

    bool    exist(const string& dir)    const;

//	virtual bool next(FILE* fp)                    = 0;

    virtual Input*  newInput(const string& name)  const = 0;
    virtual Grid* defineGridForCheck(const string& path) const = 0;

    virtual bool*   getLsmBoolValues(size_t* valuesLength)            const = 0;
//	virtual long*   getReducedGridSpecification(size_t* valuesLength) const = 0;
    virtual void getDoubleValues(const string& path, vector<double>& values)  const = 0;


    virtual void    getLatLonValues(vector<Point>& points)            const = 0;

    Field* defineField(const vector<double>& values, const FieldDescription& dc) const;
    virtual Field* defineField(const FieldDescription& dc)       const;

    virtual string       typeOf()  const = 0;


  protected:

// -- Members
    string  fileName_;

// -- Methods
    virtual void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    Input(const Input&);
    Input& operator=(const Input&);

// -- Members

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members

};

#endif

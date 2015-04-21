/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Lsm_H
#define Lsm_H

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#include "ThreadSafeMap.h"
#include <sstream>

// Forward declarations
class Input;
class Grid;

class Lsm {

  public:
// -- Contructors

    Lsm( Input* input);

// -- Destructor

    virtual ~Lsm(); // Change to virtual if base class

    virtual double  seaPoint(double latitude,double longitude) const = 0;
    virtual bool    seaPointBool(double latitude,double longitude) const = 0;

    virtual	bool    isAvailablePredefinedLsm()                       = 0;
    virtual	string  directoryOfPredefined()                    const = 0;

    virtual ref_counted_ptr< const vector<double> > getLsmValuesDouble(const Grid& gridSpec);

    virtual void   createGlobalLsmAndWriteToFileAsGrib(const Grid&) const;
    Input*          releaseInput()  {
        return input_.release();
    }

    string          path() const    {
        return path_;
    }
    string          pathForGeneratedFiles() const    {
        return pathForGeneratedFiles_;
    }

  protected:

// -- Members
    auto_ptr<Input> input_;
    string path_;
    string pathForGeneratedFiles_;

    static ThreadSafeMap<std::vector<double> > cache_;
    static string cacheKey(const Input& in, const Grid& grid);

  private:

// No copy allowed

    Lsm(const Lsm&);
    Lsm& operator=(const Lsm&);

// -- Members

};

#endif

/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef GeoPointsOutput_H
#define GeoPointsOutput_H

#include "mir/output/MIROutput.h"


namespace eckit {
class DataHandle;
}


namespace mir {
namespace output {


class GeoPointsOutput : public MIROutput {
  public:

// -- Exceptions
    // None

// -- Contructors

    GeoPointsOutput();

// -- Destructor

    ~GeoPointsOutput(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    // None

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

    virtual eckit::DataHandle& dataHandle() const = 0;

// -- Overridden methods
    // From MIROutput


    virtual size_t copy(const param::MIRParametrisation&, input::MIRInput&); // Not iterpolation performed
    virtual size_t save(const param::MIRParametrisation&, input::MIRInput&, data::MIRField&);

    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    GeoPointsOutput(const GeoPointsOutput&);
    GeoPointsOutput& operator=(const GeoPointsOutput&);

// -- Members

    bool once_;

// -- Methods
    // None


// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const GeoPointsOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif


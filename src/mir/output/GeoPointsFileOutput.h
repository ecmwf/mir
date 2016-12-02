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


#ifndef GeoPointsFileOutput_H
#define GeoPointsFileOutput_H

#include "mir/output/GeoPointsOutput.h"
#include "eckit/memory/ScopedPtr.h"



namespace mir {
namespace output {


class GeoPointsFileOutput : public GeoPointsOutput {
public:

// -- Exceptions
    // None

// -- Contructors

    GeoPointsFileOutput(const std::string& path);

// -- Destructor

    ~GeoPointsFileOutput(); // Change to virtual if base class

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


// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

private:

// No copy allowed

    GeoPointsFileOutput(const GeoPointsFileOutput&);
    GeoPointsFileOutput& operator=(const GeoPointsFileOutput&);

// -- Members

    std::string path_;
    mutable eckit::ScopedPtr<eckit::DataHandle> handle_;

// -- Methods
    // None


// -- Overridden methods
    // From MIROutput
    virtual void print(std::ostream&) const; // Change to virtual if base class
    virtual bool sameAs(const MIROutput& other) const;
    virtual eckit::DataHandle& dataHandle() const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const GeoPointsFileOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif


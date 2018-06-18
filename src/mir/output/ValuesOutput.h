/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef ValuesOutput_H
#define ValuesOutput_H

#include "mir/output/MIROutput.h"

#include <vector>

namespace eckit {
class DataHandle;
}


namespace mir {
namespace output {


class ValuesOutput : public MIROutput {
public:

// -- Exceptions
    // None

// -- Contructors

    ValuesOutput();

// -- Destructor

    ~ValuesOutput(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    const std::vector<double>& values(size_t which = 0) const;
    size_t dimensions() const;

    double missingValue() const;
    bool hasMissing() const;


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
    // From MIROutput


    virtual size_t copy(const param::MIRParametrisation &, context::Context &); // No interpolation performed
    virtual size_t save(const param::MIRParametrisation&, context::Context&);

    virtual bool sameParametrisation(const param::MIRParametrisation &param1,
                                     const param::MIRParametrisation & param2) const;

    virtual bool printParametrisation(std::ostream& out, const param::MIRParametrisation &param) const;

    // None

// -- Class members
    // None

// -- Class methods
    // None

private:

// -- Members

    std::vector<std::vector<double> > values_;
    bool hasMissing_;
    double missingValue_;

// -- Methods
    // None


// -- Overridden methods

    virtual bool sameAs(const MIROutput& other) const;
    virtual void print(std::ostream&) const;

// Change to virtual if base class

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const ValuesOutput& p)
    // { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif


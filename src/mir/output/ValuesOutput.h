/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_output_ValuesOutput_h
#define mir_output_ValuesOutput_h

#include <vector>

#include "mir/data/MIRValuesVector.h"
#include "mir/output/MIROutput.h"


namespace mir {
namespace output {


class ValuesOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Contructors

    ValuesOutput();

    // -- Destructor

    ~ValuesOutput();  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const MIRValuesVector& values(size_t which = 0) const;
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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::vector<MIRValuesVector> values_;
    double missingValue_;
    bool hasMissing_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    virtual size_t copy(const param::MIRParametrisation&, context::Context&);  // No interpolation performed
    virtual size_t save(const param::MIRParametrisation&, context::Context&);
    virtual bool sameAs(const MIROutput&) const;
    virtual bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const;
    virtual bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const;
    virtual void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace output
}  // namespace mir


#endif

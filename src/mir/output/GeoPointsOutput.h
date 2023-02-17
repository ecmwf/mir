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


#pragma once

#include "mir/output/MIROutput.h"


namespace eckit {
class DataHandle;
}


namespace mir::output {


class GeoPointsOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GeoPointsOutput();

    // -- Destructor

    ~GeoPointsOutput() override;

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

    bool once();

    virtual eckit::DataHandle& dataHandle() const = 0;

    // -- Overridden methods

    // From MIROutput
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    bool once_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output

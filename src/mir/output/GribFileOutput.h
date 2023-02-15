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

#include "eckit/filesystem/PathName.h"

#include "mir/output/GribStreamOutput.h"


namespace mir::output {


class GribFileOutput : public GribStreamOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribFileOutput(const eckit::PathName&, bool append = false);

    // -- Destructor

    ~GribFileOutput() override;

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
    // -- Members

    eckit::PathName path_;
    eckit::DataHandle* handle_;
    bool append_;

    // -- Methods
    // None

    // -- Overridden methods
    // From MIROutput
    bool sameAs(const MIROutput&) const override;
    void print(std::ostream&) const override;

    // From GribInput

    eckit::DataHandle& dataHandle() override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output

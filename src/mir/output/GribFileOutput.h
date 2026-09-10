// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

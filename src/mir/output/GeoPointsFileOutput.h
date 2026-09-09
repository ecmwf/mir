// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "mir/output/GeoPointsOutput.h"


namespace mir::output {


class GeoPointsFileOutput : public GeoPointsOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GeoPointsFileOutput(const std::string& path, bool binary);

    // -- Destructor

    ~GeoPointsFileOutput() override;

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

    std::string path_;
    mutable std::unique_ptr<eckit::DataHandle> handle_;
    bool binary_;

    // -- Methods
    // None

    // -- Overridden methods

    // From GeoPointsOutput
    eckit::DataHandle& dataHandle() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    void print(std::ostream&) const override;
    bool sameAs(const MIROutput&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output

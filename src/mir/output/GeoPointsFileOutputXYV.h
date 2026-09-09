// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/output/GeoPointsFileOutput.h"


namespace mir::output {


class GeoPointsFileOutputXYV : public GeoPointsFileOutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GeoPointsFileOutputXYV(const std::string& path, bool binary = false);

    // -- Destructor
    // None

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
    // None

    // -- Overridden methods

    // From MIROutput
    size_t save(const param::MIRParametrisation&, context::Context&) override;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    size_t saveText(const param::MIRParametrisation&, context::Context&);
    size_t saveBinary(const param::MIRParametrisation&, context::Context&);

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

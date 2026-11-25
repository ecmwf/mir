// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/compat/GribCompatibility.h"


namespace mir::compat {


class Grib1DeleteLocalDefinition final : public GribCompatibility {
public:
    explicit Grib1DeleteLocalDefinition(const std::string& name) : GribCompatibility(name) {}

private:
    void execute(const output::MIROutput&, const param::MIRParametrisation&, grib_handle*, grib_info&) const override;
    void printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    void initialise(const metkit::mars::MarsRequest&, std::map<std::string, std::string>& postproc) const override;
    void print(std::ostream&) const override;
};


}  // namespace mir::compat

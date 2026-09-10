// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <vector>


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::util {


class GlobaliseUnstructured {
public:
    // -- Constructors

    GlobaliseUnstructured(const param::MIRParametrisation&);

    // -- Destructor

    ~GlobaliseUnstructured();

    // -- Methods

    size_t appendGlobalPoints(std::vector<double>& latitudes, std::vector<double>& longitudes);

private:
    // -- Members

    const param::MIRParametrisation& parametrisation_;
    std::string globaliseGridname_;
    double globaliseMissingRadius_;
};


}  // namespace mir::util

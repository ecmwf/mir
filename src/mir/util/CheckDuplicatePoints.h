// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <vector>


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::util {


void check_duplicate_points(const std::string& title, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes);


void check_duplicate_points(const std::string& title, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes, const param::MIRParametrisation&);


}  // namespace mir::util

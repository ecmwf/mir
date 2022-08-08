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

#include <vector>


namespace mir {
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir {
namespace util {


void check_duplicate_points(const std::string& title, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes);


void check_duplicate_points(const std::string& title, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes, const param::MIRParametrisation&);


}  // namespace util
}  // namespace mir

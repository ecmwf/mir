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

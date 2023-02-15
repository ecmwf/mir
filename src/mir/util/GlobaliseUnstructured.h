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

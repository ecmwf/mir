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


#ifndef mir_input_GlobaliseUnstructured_h
#define mir_input_GlobaliseUnstructured_h

#include <string>
#include <vector>


namespace mir {
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir {
namespace util {


class GlobaliseUnstructured {
public:
    // -- Contructors

    GlobaliseUnstructured(const param::MIRParametrisation&);

    // -- Destructor

    ~GlobaliseUnstructured();  // Change to virtual if base class

    // -- Methods

    size_t appendGlobalPoints(std::vector<double>& latitudes, std::vector<double>& longitudes);

private:
    // -- Members

    const param::MIRParametrisation& parametrisation_;
    std::string globaliseGridname_;
    double globaliseMissingRadius_;
};


}  // namespace util
}  // namespace mir


#endif

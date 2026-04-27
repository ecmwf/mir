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


struct grib_info;

namespace mir {
namespace api {
class MIRJob;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::util {


struct ScanningMode {
    static const std::string& default_scanning_mode();

    explicit ScanningMode(const param::MIRParametrisation&);
    explicit ScanningMode(const std::string& order) : order_(order) {}

    const std::string& order() const { return order_; }

    void fillGrib(grib_info&) const;
    void fillJob(api::MIRJob&) const;

private:
    std::string order_;
};


}  // namespace mir::util

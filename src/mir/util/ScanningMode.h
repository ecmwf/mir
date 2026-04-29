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

#include "eckit/geo/order/Scan.h"


struct grib_info;

namespace eckit::spec {
class Custom;
}  // namespace eckit::spec

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
    explicit ScanningMode(const std::string& order);

    const std::string& order() const { return scan_.order(); }

    bool iScansNegatively() const { return !scan_.is_scan_i_positive(); }
    bool iScansPositively() const { return scan_.is_scan_i_positive(); }
    bool jScansNegatively() const { return !scan_.is_scan_j_positive(); }
    bool jScansPositively() const { return scan_.is_scan_j_positive(); }
    bool jPointsAreConsecutive() const { return !scan_.is_scan_i_then_j(); }
    bool alternativeRowScanning() const { return scan_.is_scan_alternating(); }

    void fillGrib(grib_info&) const;
    void fillJob(api::MIRJob&) const;
    void fillSpec(eckit::spec::Custom&) const;

private:
    eckit::geo::order::Scan scan_;
};


}  // namespace mir::util

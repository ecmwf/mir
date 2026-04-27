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


#include "mir/util/ScanningMode.h"

#include "eckit/geo/order/Scan.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir::util {


const std::string& ScanningMode::default_scanning_mode() {
    return eckit::geo::order::Scan::order_default();
}


ScanningMode::ScanningMode(const param::MIRParametrisation& param) :
    order_([&param]() {
        auto order = eckit::geo::order::Scan::order_default();
        param.get("order", order);
        return order;
    }()) {}


void ScanningMode::fillGrib(grib_info& info) const {
    eckit::geo::order::Scan scan(order_);

    info.grid.iScansNegatively = scan.is_scan_i_positive() ? 0 : 1;
    info.grid.jScansPositively = scan.is_scan_j_positive() ? 1 : 0;

    info.extra_set("scanningMode", grib_order_to_scanning_mode(order_));
}


void ScanningMode::fillJob(api::MIRJob& job) const {
    job.set("order", order_);
}


}  // namespace mir::util

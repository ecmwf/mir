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

#include "eckit/spec/Custom.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir::util {


const std::string& ScanningMode::default_scanning_mode() {
    return eckit::geo::order::Scan::order_default();
}


ScanningMode::ScanningMode(const param::MIRParametrisation& param) :
    ScanningMode([&param]() {
        auto order = eckit::geo::order::Scan::order_default();
        param.get("order", order);
        return order;
    }()) {}


ScanningMode::ScanningMode(const std::string& order) : scan_(order) {}


void ScanningMode::fillGrib(grib_info& info) const {
    info.grid.iScansNegatively = scan_.is_scan_i_positive() ? 0 : 1;
    info.grid.jScansPositively = scan_.is_scan_j_positive() ? 1 : 0;

    info.extra_set("scanningMode", grib_order_to_scanning_mode(scan_.order()));
}


void ScanningMode::fillJob(api::MIRJob& job) const {
    job.set("order", scan_.order());
}


void ScanningMode::fillSpec(eckit::spec::Custom& spec) const {
    spec.set("order", scan_.order());
}


}  // namespace mir::util

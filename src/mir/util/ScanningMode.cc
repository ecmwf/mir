// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
    if (scan_.is_scan_i_positive()) {
        info.grid.iScansNegatively = 0;
    }
    else {
        info.grid.iScansNegatively = 1;
        std::swap(info.grid.longitudeOfFirstGridPointInDegrees, info.grid.longitudeOfLastGridPointInDegrees);
    }

    if (scan_.is_scan_j_positive()) {
        info.grid.jScansPositively = 1;
        std::swap(info.grid.latitudeOfFirstGridPointInDegrees, info.grid.latitudeOfLastGridPointInDegrees);
    }
    else {
        info.grid.jScansPositively = 0;
    }

    info.extra_set("scanningMode", grib_order_to_scanning_mode(scan_.order()));
}


void ScanningMode::fillJob(api::MIRJob& job) const {
    job.set("order", scan_.order());
}


void ScanningMode::fillSpec(eckit::spec::Custom& spec) const {
    spec.set("order", scan_.order());
}


}  // namespace mir::util

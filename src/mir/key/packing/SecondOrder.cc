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


#include "mir/key/packing/SecondOrder.h"

#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace packing {


static const PackingBuilder<SecondOrder> __packing("second-order", "so", false, true);


static bool check(const repres::Representation* repres) {
    ASSERT(repres != nullptr);

    auto n = repres->numberOfPoints();
    if (n < 4) {
        Log::warning() << "packing=second-order: does not support less than 4 values, using packing=simple"
                       << std::endl;
        return false;
    }
    return true;
}


SecondOrder::SecondOrder(const std::string& name, const param::MIRParametrisation& param) :
    Packing(name, param), simple_(name, param) {}


void SecondOrder::fill(const repres::Representation* repres, grib_info& info) const {
    if (!check(repres)) {
        simple_.fill(repres, info);
        return;
    }

    Packing::fill(info, CODES_UTIL_PACKING_TYPE_GRID_SECOND_ORDER);
}


void SecondOrder::set(const repres::Representation* repres, grib_handle* handle) const {
    if (!check(repres)) {
        simple_.set(repres, handle);
        return;
    }

    Packing::set(handle, "grid_second_order");
}


}  // namespace packing
}  // namespace key
}  // namespace mir

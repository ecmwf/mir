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


#include "mir/key/packing/SimpleMatrix.h"

#include <ostream>

#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<SimpleMatrix> __packing("simple-matrix", false, true);


void SimpleMatrix::fill(grib_info& info) const {
    ASSERT(!definePacking_);
    saveAccuracy(info);
    saveEdition(info);
}


void SimpleMatrix::set(grib_handle* handle) const {
    ASSERT(gridded());
    setPacking(handle, "grid_simple_matrix");
    setAccuracy(handle);
    setEdition(handle);
}


void SimpleMatrix::print(std::ostream& out) const {
    out << "SimpleMatrix[]";
}


}  // namespace packing
}  // namespace key
}  // namespace mir

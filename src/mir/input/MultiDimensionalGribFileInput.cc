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


#include "mir/input/MultiDimensionalGribFileInput.h"

#include "mir/input/GribFileInput.h"


namespace mir::input {


MultiDimensionalGribFileInput::MultiDimensionalGribFileInput(const eckit::PathName& path, size_t dim, size_t skip) {
    for (size_t which = 0; which < dim; ++which) {
        append(new GribFileInput(path, skip + which, dim));
    }
}


}  // namespace mir::input

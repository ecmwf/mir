// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/MultiDimensionalGribFileInput.h"

#include "mir/input/GribFileInput.h"


namespace mir::input {


MultiDimensionalGribFileInput::MultiDimensionalGribFileInput(const eckit::PathName& path, size_t dim, size_t skip) {
    for (size_t which = 0; which < dim; ++which) {
        append(new GribFileInput(path, skip + which, dim));
    }
}


}  // namespace mir::input

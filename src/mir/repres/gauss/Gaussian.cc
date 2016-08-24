/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/repres/gauss/Gaussian.h"

#include <map>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Timer.h"
#include "atlas/grid/Domain.h"
#include "atlas/grid/gaussian/latitudes/Latitudes.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace repres {


Gaussian::Gaussian(size_t N) :
    N_(N) {
}


Gaussian::Gaussian(size_t N, const util::BoundingBox &bbox) :
    N_(N),
    bbox_(bbox) {
}


Gaussian::Gaussian(const param::MIRParametrisation &parametrisation) :
    bbox_(parametrisation) {
    ASSERT(parametrisation.get("N", N_));
}


Gaussian::~Gaussian() {
}


std::vector<double> Gaussian::latitudes(size_t N) {
    std::vector<double> latitudes(2 * N);
    atlas::grid::gaussian::latitudes::gaussian_latitudes_npole_spole(N, &latitudes[0]);
    return latitudes;
}


const std::vector<double>& Gaussian::latitudes() const {
    // This returns the Gaussian latitudes of a GLOBAL field
    if (latitudes_.size() == 0) {
        latitudes_.resize(N_ * 2);
        atlas::grid::gaussian::latitudes::gaussian_latitudes_npole_spole(N_, &latitudes_[0]);
    }
    return latitudes_;
}


}  // namespace repres
}  // namespace mir


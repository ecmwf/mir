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
#include "eckit/types/FloatCompare.h"
#include "atlas/grid/Domain.h"
#include "atlas/grid/gaussian/latitudes/Latitudes.h"
#include "mir/config/LibMir.h"
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

#if 0
        // Make sure calculated latitudes comply with BoundingBox, enlarging it if necessary
        double lat_max = latitudes_.front();
        double max_inc_north_south = std::numeric_limits<double>::epsilon();
        for (size_t j=1; j<latitudes_.size(); ++j) {
            max_inc_north_south = std::max(max_inc_north_south, latitudes_[j-1]-latitudes_[j]);
        }

        typedef eckit::FloatCompare<double> cmp;
        const util::BoundingBox bbox_larger(
                    cmp::isApproximatelyEqual(bbox_.north(),  90, max_inc_north_south)? std::max(bbox_.north(),  lat_max) : bbox_.north(), bbox_.west(),
                    cmp::isApproximatelyEqual(bbox_.south(), -90, max_inc_north_south)? std::min(bbox_.south(), -lat_max) : bbox_.south(), bbox_.east() );

        if (bbox_!=bbox_larger) {
            std::streamsize prev = eckit::Log::trace<MIR>().precision(12);
            eckit::Log::trace<MIR>() << "BoundingBox: " << bbox_ << " enlarged to " << bbox_larger << std::endl;
            eckit::Log::trace<MIR>().precision(prev);
            const_cast< util::BoundingBox& >(this->bbox_) = bbox_larger;
        }
#endif
    }
    return latitudes_;
}


}  // namespace repres
}  // namespace mir


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

#include "mir/method/voronoi/VoronoiMethod.h"


namespace mir {
namespace method {
namespace voronoi {


struct VoronoiAverage final : VoronoiMethod {
    explicit VoronoiAverage(const param::MIRParametrisation&);
    VoronoiAverage(const VoronoiAverage&) = delete;
    void operator=(const VoronoiAverage&) = delete;

    // FIXME temporary
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
};


}  // namespace voronoi
}  // namespace method
}  // namespace mir

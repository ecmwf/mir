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


#ifndef mir_method_voronoi_StatisticsT_h
#define mir_method_voronoi_StatisticsT_h

#include "mir/method/voronoi/VoronoiMethod.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace voronoi {


template <typename T>
struct StatisticsT final : VoronoiMethod {
    explicit StatisticsT(const param::MIRParametrisation&);
    StatisticsT(const StatisticsT&) = delete;
    void operator=(const StatisticsT&) = delete;
    const char* name() const override { NOTIMP; /*ensure specialization*/ }
};


}  // namespace voronoi
}  // namespace method
}  // namespace mir


#endif

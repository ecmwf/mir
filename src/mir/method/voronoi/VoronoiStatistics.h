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


namespace mir::method::voronoi {


struct VoronoiStatistics final : VoronoiMethod {
    explicit VoronoiStatistics(const param::MIRParametrisation&);

    const char* type() const override;
    void json(eckit::JSON&) const override;

private:
    std::string interpolationStatistics_;
};


}  // namespace mir::method::voronoi

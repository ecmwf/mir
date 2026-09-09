// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

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


#include "mir/util/Types.h"

#include <algorithm>

#include "eckit/utils/Translator.h"

#include "mir/util/Angles.h"
#include "mir/util/Grib.h"


namespace atlas {


bool projection::ProjectionFactory::has(const std::string&) {
    return false;
}


void util::gaussian_latitudes_npole_spole(size_t N, double* latitudes) {
    codes_get_gaussian_latitudes(long(N), latitudes);
}


void util::gaussian_quadrature_npole_spole(size_t /*N*/, double* /*latitudes*/, double* /*weights*/) {
    // used in interpolation=grid-box-average/maximum
    NOTIMP;
}


namespace grid {
LinearSpacing::LinearSpacing(value_type /*a*/, value_type /*b*/, long n, bool /*endpoint*/) : Spacing(n) {
    NOTIMP;
}
}  // namespace grid


StructuredGrid::StructuredGrid(const std::string& name, const Domain& domain) {
    ASSERT(domain.north() == globalDomain.north());
    ASSERT(domain.south() == globalDomain.south());
    ASSERT(domain.west() == globalDomain.west());
    ASSERT(domain.east() == globalDomain.east());

    ASSERT(name.size() > 1);
    auto c = name.front();
    auto n = eckit::Translator<std::string, idx_t>()(name.substr(1));
    ASSERT(n > 0);

    if (c == 'F' || c == 'f') {
        pl_.assign(size_t(2 * n), 4 * n);
        return;
    }

    if (c == 'O' || c == 'o') {
        pl_.resize(size_t(2 * n));

        auto a = pl_.begin();
        auto b = pl_.rbegin();
        for (idx_t j = 0, ni = 20; j < n; j++, ni += 4) {
            *(a++) = *(b++) = ni;
        }
        return;
    }

    NOTIMP;
}


idx_t StructuredGrid::nx() const {
    auto mm = std::minmax_element(pl_.begin(), pl_.end());
    return *mm.first == *mm.second ? *mm.first : NOTIMP;
}


}  // namespace atlas

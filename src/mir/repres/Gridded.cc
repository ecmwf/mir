/*
 * (C) Copyright 1996-2015 ECMWF.
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


#include "mir/repres/Gridded.h"
#include "mir/util/Grib.h"
#include "eckit/log/Timer.h"
#include "mir/util/BoundingBox.h"
#include "mir/repres/Iterator.h"

#include <map>

namespace mir {
namespace repres {


Gridded::Gridded() {}


Gridded::Gridded(const param::MIRParametrisation &parametrisation) {}


Gridded::~Gridded() {}

void Gridded::setComplexPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_COMPLEX;
}

void Gridded::setSimplePacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SIMPLE;
}

void Gridded::setSecondOrderPacking(grib_info &info) const {
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_GRID_SECOND_ORDER;
}


Iterator *Gridded::iterator() const {
    eckit::StrStream os;
    os << "Gridded::iterator() not implemented for " << *this << eckit::StrStream::ends;
    throw eckit::SeriousBug(std::string(os));
}


Gridded *Gridded::cropped(const util::BoundingBox &bbox) const {
    eckit::StrStream os;
    os << "Gridded::cropped() not implemented for " << *this << eckit::StrStream::ends;
    throw eckit::SeriousBug(std::string(os));
}

Representation *Gridded::crop(const util::BoundingBox &bbox, const std::vector<double> &in, std::vector<double> &out) const {

    eckit::Timer timer("crop");

    struct LL {
        double lat_;
        double lon_;
        LL(double lat, double lon): lat_(lat), lon_(lon) {}
        bool operator<(const LL &other) const {
            // Order must be like natural scanning mode
            if (lat_ == other.lat_) {
                return lon_ < other.lon_;
            }

            return lat_ > other.lat_;
        }
    };

    // TODO: Consider caching these maps (e.g. cache map LL -> index instead)
    std::map<LL, double> m;

    validate(in);

    double n = 0;
    double s = 0;
    double e = 0;
    double w = 0;


    size_t p = 0;
    bool first = true;
    double lat, lon;
    std::auto_ptr<Iterator> iter(iterator());
    while (iter->next(lat, lon)) {
        if (bbox.contains(lat, lon)) {

            lon = bbox.normalise(lon);

            if (first) {
                n = s = lat;
                w = e = lon;
                first = false;
            } else {
                n = std::max(n, lat);
                s = std::min(s, lat);
                e = std::max(e, lon);
                w = std::min(w, lon);
            }

            m.insert(std::make_pair(LL(lat, lon), in[p]));

        }
        p++;
    }


    out.clear();
    out.reserve(m.size());

    for (std::map<LL, double>::const_iterator j = m.begin(); j != m.end(); ++j) {
        out.push_back((*j).second);
    }

    eckit::Log::info() << "CROP resulting bbox is: " << util::BoundingBox(n, w, s, e) <<
                       ", size=" << out.size() << std::endl;
    Gridded *cropped =  this->cropped(util::BoundingBox(n, w, s, e));
    eckit::Log::info() << *cropped << std::endl;

    ASSERT(out.size() > 0);
    cropped->validate(out);
    // ASSERT(cropped->ni() * cropped->nj() == out.size());
    ASSERT(p == in.size());

    return cropped;
}
}  // namespace repres
}  // namespace mir


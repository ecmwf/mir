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

#include <map>
#include <cmath>

#include "eckit/log/Timer.h"
#include "eckit/memory/ScopedPtr.h"

#include "mir/util/Grib.h"
#include "mir/util/BoundingBox.h"
#include "mir/repres/Iterator.h"

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


Iterator *Gridded::iterator(bool unrotated) const {
    eckit::StrStream os;
    os << "Gridded::iterator(bool unrotated) not implemented for " << *this << eckit::StrStream::ends;
    throw eckit::SeriousBug(std::string(os));
}


const Gridded *Gridded::cropped(const util::BoundingBox &bbox) const {
    eckit::StrStream os;
    os << "Gridded::cropped() not implemented for " << *this << eckit::StrStream::ends;
    throw eckit::SeriousBug(std::string(os));
}

bool Gridded::globalDomain() const {
    eckit::StrStream os;
    os << "Representation::globalDomain() not implemented for " << *this << eckit::StrStream::ends;
    throw eckit::SeriousBug(std::string(os));
}

void Gridded::cropToDomain(const param::MIRParametrisation &parametrisation, data::MIRField &field) const {

    if (!globalDomain()) {
        Representation::cropToDomain(parametrisation, field); // This will throw an exception
    }
}

const Gridded *Gridded::crop(const util::BoundingBox &bbox, const std::vector<double> &in, std::vector<double> &out) const {

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
    size_t count = 0;
    bool first = true;
    double lat, lon;

    // Iterator is "unrotated", because the cropping area
    // is expressed in the rotated coordinated system
    eckit::ScopedPtr<Iterator> iter(iterator(true));
    while (iter->next(lat, lon)) {
        // std::cout << lat << " " << lon << std::endl;
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

            // if(m.find(LL(lat, lon)) != m.end()) {
            //     eckit::Log::info() << "CROP  duplicate " << lat << ", " << lon << std::endl;
            // }
            m.insert(std::make_pair(LL(lat, lon), in[p]));
            count++;

        }
        p++;
    }

    // Make sure we did not visit duplicate points
    eckit::Log::info() << "CROP inserted points " << count << ", unique points " << m.size() << std::endl;
    ASSERT(count == m.size());

    out.clear();
    out.reserve(m.size());

    for (std::map<LL, double>::const_iterator j = m.begin(); j != m.end(); ++j) {
        out.push_back((*j).second);
    }

    eckit::Log::info() << "CROP resulting bbox is: " << util::BoundingBox(n, w, s, e) <<
                       ", size=" << out.size() << std::endl;
    const Gridded *cropped =  this->cropped(util::BoundingBox(n, w, s, e));
    eckit::Log::info() << *cropped << std::endl;

    ASSERT(out.size() > 0);
    cropped->validate(out);
    // ASSERT(cropped->ni() * cropped->nj() == out.size());
    eckit::Log::info() << "CROP p=" << p << " size=" << in.size() << std::endl;
    ASSERT(p == in.size());

    return cropped;
}


void Gridded::checkerboard(std::vector<double> &values, bool hasMissing, double missingValue) const {


    double minvalue = 0;
    double maxvalue = 0;

    size_t first   = 0;
    for (; first < values.size(); ++first) {
        if (!hasMissing || values[first] != missingValue) {
            minvalue = values[first];
            maxvalue = values[first];
            break;
        }
    }

    if (first == values.size()) {
        // Only missing values
        return;
    }

    for (size_t i = first; i < values.size(); ++i) {
        if (!hasMissing || values[i] != missingValue) {
            minvalue = std::min(minvalue, values[i]);
            maxvalue = std::max(maxvalue, values[i]);
        }
    }

    size_t we = 16;
    size_t ns = 8;

    double dwe = 360.0 / we;
    double dns = 180.0 / ns;



    // Assumes iterator scans in the same order as the values
    eckit::ScopedPtr<Iterator> iter(iterator(false));
    double lat = 0;
    double lon = 0;

    std::vector<double> v;
    v.push_back(minvalue);
    v.push_back(maxvalue);

    std::map<std::pair<size_t, size_t>, size_t> boxes;

    size_t b = 0;
    for (size_t r = 0; r < we; r++) {
        for (size_t c = 0; c < ns; c++) {
            boxes[std::make_pair(r, c)] = b;
            b++;
            b %= v.size();
        }
        b++;
        b %= v.size();
    }


    size_t k = 0;

    while (iter->next(lat, lon)) {

        lat = 90 - lat;

        while (lon >= 369) {
            lon -= 360;
        }

        while (lon < 0) {
            lon += 360;
        }

        size_t c = size_t(lat / dns);
        size_t r = size_t(lon / dwe);

        if (!hasMissing || values[k] != missingValue) {
            values[k] = boxes[std::make_pair(r, c)];
        }

        k++;
    }

    ASSERT(k == values.size());
}

void Gridded::pattern(std::vector<double> &values, bool hasMissing, double missingValue) const {


    double minvalue = 0;
    double maxvalue = 0;

    size_t first = 0;
    size_t count = 0;
    for (; first < values.size(); ++first) {
        if (!hasMissing || values[first] != missingValue) {
            minvalue = values[first];
            maxvalue = values[first];
            count++;
            break;
        }
    }

    if (first == values.size()) {
        // Only missing values
        return;
    }

    for (size_t i = first; i < values.size(); ++i) {
        if (!hasMissing || values[i] != missingValue) {
            minvalue = std::min(minvalue, values[i]);
            maxvalue = std::max(maxvalue, values[i]);
            count++;
        }
    }

    double median = (minvalue + maxvalue) / 2;
    double range = maxvalue - minvalue;

    eckit::ScopedPtr<Iterator> iter(iterator(false));
    double lat = 0;
    double lon = 0;

    std::vector<double> v;
    v.push_back(minvalue);
    v.push_back(maxvalue);


    size_t k = 0;
    const double deg2rad = M_PI/180.0;

    while (iter->next(lat, lon)) {

        if (!hasMissing || values[k] != missingValue) {
            values[k] =
                range * sin(3 * lon * deg2rad) * cos(lat * deg2rad * 3) * 0.5 + median;
        }

        k++;
    }

    ASSERT(k == values.size());
}

size_t Gridded::computeN(double first, double last, double inc, const char *n_name, const char *first_name, const char *last_name) {
    size_t n;
    if (!(first <= last)) {
        eckit::Log::info() << first_name << " (first):" << first << ", " << last_name << " (last)" << last << std::endl;
        ASSERT(first <= last);
    }
    ASSERT(inc > 0);
    size_t p = size_t((last - first) / inc);
    double d0 = fabs(last - (first + p * inc));
    double d1 = fabs(last - (first + (p + 1) * inc));

    // eckit::Log::info() << p << " " << d0 << " " << d1 << " " << inc << " " << first << " " << last << std::endl;
    ASSERT(d0 != d1);

    if (d0 < d1) {
        n = p;
    } else {
        n = p + 1;
    }

    if ((n * inc + first) != last) {
        eckit::Log::info() << "Gridded: cannot compute accuratly "
                           << n_name << ", given "
                           << first_name << "=" << first << ", "
                           << last_name << "=" << last << " and increment=" << inc << std::endl;
        eckit::Log::info() << "Last value is computed as " << (p * inc + first)
                           << ", diff=" << (last - (p * inc + first))
                           << std::endl;
    }

    return n + 1;

}
}  // namespace repres
}  // namespace mir


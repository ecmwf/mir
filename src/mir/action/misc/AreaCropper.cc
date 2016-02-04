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

#include "mir/action/misc/AreaCropper.h"

#include <iostream>
#include <vector>

#include "atlas/Grid.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/log/Timer.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/caching/CroppingCache.h"


namespace mir {
namespace action {

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




static eckit::Mutex local_mutex;
static std::map< std::string, caching::CroppingCacheEntry > cache;


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation):
    Action(parametrisation),
    bbox_(),
    caching_(true) {

    std::vector<double> value;
    ASSERT(parametrisation.get("user.area", value));
    ASSERT(value.size() == 4);

    parametrisation_.get("caching", caching_);

    bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
}


AreaCropper::AreaCropper(const param::MIRParametrisation &parametrisation, const util::BoundingBox &bbox):
    Action(parametrisation),
    bbox_(bbox),
    caching_(true) {
}

AreaCropper::~AreaCropper() {
}


void AreaCropper::print(std::ostream &out) const {
    out << "AreaCropper[bbox=" << bbox_ << "]";
}

// TODO: Write cache to disk
static const caching::CroppingCacheEntry &getMapping(const repres::Representation *representation,
        const util::BoundingBox &bbox,
        bool caching) {

    eckit::ScopedPtr<atlas::Grid> gin(representation->atlasGrid()); // This should disapear once we move Representation to atlas
    eckit::MD5 md5;
    md5 << *gin << bbox;

    std::string key(md5);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    caching::CroppingCacheEntry &c = cache[key];
    if (c.mapping_.size() > 0) {
        return c;
    }

    static caching::CroppingCache disk;

    if (caching && disk.retrieve(key, c)) {
        return c;
    }

    eckit::Timer timer("Compute crop mapping");

    // TODO: Consider caching these maps (e.g. cache map LL -> index instead)
    std::map<LL, size_t> m;

    double n = 0;
    double s = 0;
    double e = 0;
    double w = 0;

    size_t p = 0;
    size_t count = 0;
    bool first = true;
    double lat, lon;

    // Iterator is "unrotated", because the cropping area
    // is expressed in before the rotation is applied
    eckit::ScopedPtr<repres::Iterator> iter(representation->unrotatedIterator());
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
            m.insert(std::make_pair(LL(lat, lon), p));
            count++;

        }
        p++;
    }

    // Make sure we did not visit duplicate points
    eckit::Log::info() << "CROP inserted points " << count << ", unique points " << m.size() << std::endl;
    ASSERT(count == m.size());

    // Don't support empty results
    ASSERT(m.size() > 0);

    c.bbox_ = util::BoundingBox(n, w, s, e);
    c.mapping_.reserve(m.size());

    for (std::map<LL, size_t>::const_iterator j = m.begin(); j != m.end(); ++j) {
        c.mapping_.push_back((*j).second);
    }

    if (caching) {
        disk.insert(key, c);
    }

    return c;
}

void AreaCropper::execute(data::MIRField &field) const {

    // Keep a pointer on the original representation, as the one in the field will
    // be changed in the loop
    repres::RepresentationHandle representation(field.representation());
    const caching::CroppingCacheEntry &c = getMapping(representation, bbox_, caching_);

    eckit::Log::info() << "CROP resulting bbox is: " << c.bbox_ <<
                       ", size=" << c.mapping_.size() << std::endl;

    for (size_t i = 0; i < field.dimensions(); i++) {
        const std::vector<double> &values = field.values(i);

        std::vector<double> result;

        result.reserve(c.mapping_.size());

        for (std::vector<size_t>::const_iterator j = c.mapping_.begin(); j != c.mapping_.end(); ++j) {
            result.push_back(values[*j]);
        }

        const repres::Representation *cropped =  representation->cropped(c.bbox_);
        eckit::Log::info() << *cropped << std::endl;

        ASSERT(result.size() > 0);
        cropped->validate(result);

        field.representation(cropped);
        field.values(result, i);
    }
}


namespace {
static ActionBuilder< AreaCropper > subAreaCropper("crop.area");
}


}  // namespace action
}  // namespace mir


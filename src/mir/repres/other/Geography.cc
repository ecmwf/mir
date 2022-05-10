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


#include "mir/repres/other/Geography.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/utils/MD5.h"

#include "mir/geography/GeoJSON.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace repres {
namespace other {


Geography::Geography(const param::MIRParametrisation& param) : geometry_("lonlat") {
    // Validate geometry_
    param.get("geography-geometry", geometry_);
    auto g = geography::GeographyInput::stringAsGeometry(geometry_);

    // Generate regions (from polygons)
    std::string j;
    regions_ = (param.get("geography-geojson", j)        ? geography::GeoJSON(j, g)
                : param.get("geography-geojson-file", j) ? geography::GeoJSON(eckit::PathName(j), g)
                                                         : NOTIMP)
                   .polygons();

    // Generate regions indentifier
    eckit::MD5 h;
    h << geometry_;

    for (size_t i = 0; i < regions_.size(); ++i) {
        h << i;
        for (size_t j = 0; j < regions_[i].size(); ++j) {
            h << j;
            for (size_t k = 0; k < regions_[i][j]->size(); ++k) {
                h << k;
                const auto& p = (*regions_[i][j])[k];
                h << p.x();
                h << p.y();
            }
        }
    }
    id_ = h.digest();
}


bool Geography::inRegion(const Point2& p, size_t& index) const {
    const Point2 q{p[1], p[0]};

    // Note: one region per point, overlapping region polygons cancel out
    for (size_t i = 0; i < regions_.size(); ++i) {
        bool inside = false;
        for (const auto& p : regions_[i]) {
            inside = inside != p->contains(q);
        }

        if (inside) {
            index = i;
            return true;
        }
    }

    return false;
}


void Geography::hash(eckit::MD5& h) const {
    h << id_;  // a double hash, but convenient
}


void Geography::print(std::ostream& out) const {
    out << "Geography[regions=" << numberOfPoints() << ",geometry=" << geometry_ << ",id=" << id_ << "]";
}


void Geography::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("Geography", values.size(), numberOfPoints());
}


void Geography::makeName(std::ostream& out) const {
    out << "geo-" << numberOfPoints() << "-" << geometry_ << "-" << id_;
}


bool Geography::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Geography*>(&other);
    return o != nullptr && id_ == o->id_;
}


const util::BoundingBox& Geography::boundingBox() const {
    return bbox_;
}


size_t Geography::numberOfPoints() const {
    return regions_.size();
}


}  // namespace other
}  // namespace repres
}  // namespace mir

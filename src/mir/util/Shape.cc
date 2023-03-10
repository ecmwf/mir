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


#include "mir/util/Shape.h"

#include "mir/param/MIRParametrisation.h"


namespace mir::util {


Shape::Shape(const param::MIRParametrisation& param) {
    param.get("edition", edition = 0);

    provided = param.get("shapeOfTheEarth", code = 6);

    bool isOblate = false;
    param.get("earthIsOblate", isOblate);
    param.get(isOblate ? "earthMajorAxis" : "radius", a = util::Earth::radius());
    param.get(isOblate ? "earthMinorAxis" : "radius", b = util::Earth::radius());
}


Shape::Shape(const Projection::Spec& spec) : edition(0) {
    if (spec.has("radius")) {
        code = 1L;
        a = b = spec.getDouble("radius");
        return;
    }

    if (spec.has("semi_major_axis") && spec.has("semi_minor_axis")) {
        code = 7L;
        a    = spec.getDouble("semi_major_axis");
        b    = spec.getDouble("semi_minor_axis");
        return;
    }

    std::ostringstream s;
    s << "RegularGrid: couldn't determine shape of the Earth from projection: " << spec;
    throw exception::SeriousBug(s.str());
}


Shape::Shape(const Shape& other) = default;


Shape& Shape::operator=(const Shape& other) = default;


void Shape::fillGrib(grib_info& info, const Projection::Spec& spec) const {
    // GRIB2 encoding of user-provided shape
    if (edition != 2) {
        return;
    }

    // shape given by radius or semi-major/minor axis
    if (provided) {
        info.extra_set("shapeOfTheEarth", code);
        switch (code) {
            case 1:
                info.extra_set("radius", spec.getDouble("radius", a));
                break;
            case 3:
                info.extra_set("earthMajorAxis", spec.getDouble("semi_major_axis", a) / 1000.);
                info.extra_set("earthMinorAxis", spec.getDouble("semi_minor_axis", b) / 1000.);
                break;
            case 7:
                info.extra_set("earthMajorAxis", spec.getDouble("semi_major_axis", a));
                info.extra_set("earthMinorAxis", spec.getDouble("semi_minor_axis", b));
                break;
            default:
                break;
        }
    }
    else if (spec.has("radius")) {
        info.extra_set("shapeOfTheEarth", 1L);
        info.extra_set("radius", spec.getDouble("radius"));
    }
    else if (spec.has("semi_major_axis") && spec.has("semi_minor_axis")) {
        info.extra_set("shapeOfTheEarth", 7L);
        info.extra_set("earthMajorAxis", spec.getDouble("semi_major_axis"));
        info.extra_set("earthMinorAxis", spec.getDouble("semi_minor_axis"));
    }
}


}  // namespace mir::util

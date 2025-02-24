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

#include <sstream>

#include "eckit/geo/Spec.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Earth.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


Shape::Shape(const param::MIRParametrisation& param) {
    provided = param.get("shapeOfTheEarth", code = 6);

    bool isOblate = false;
    param.get("earthIsOblate", isOblate);
    param.get(isOblate ? "earthMajorAxis" : "radius", a = util::Earth::radius());
    param.get(isOblate ? "earthMinorAxis" : "radius", b = util::Earth::radius());
}


Shape::Shape(const Projection& proj) {
    const auto& spec = proj.spec();

    if (spec.has("radius")) {
        code = 1L;
        a = b = spec.get_double("radius");
        return;
    }

    if (spec.has("semi_major_axis") && spec.has("semi_minor_axis")) {
        code = 7L;
        a    = spec.get_double("semi_major_axis");
        b    = spec.get_double("semi_minor_axis");
        return;
    }

    std::ostringstream s;
    s << "RegularGrid: couldn't determine shape of the Earth from projection: " << spec;
    throw exception::SeriousBug(s.str());
}


void Shape::fillGrib(grib_info& info, const Projection& proj) const {
    const static std::string EDITION{"edition"};

    // GRIB2 encoding of user-provided shape
    auto edition = info.packing.editionNumber;

    for (long j = 0; j < info.packing.extra_settings_count; ++j) {
        const auto& set = info.packing.extra_settings[j];
        if (set.name == EDITION && set.type == CODES_TYPE_LONG) {
            edition = set.long_value;
            break;
        }
    }

    if (edition != 2) {
        return;
    }

    // shape given by radius or semi-major/minor axis
    if (const auto& spec = proj.spec(); provided) {
        info.extra_set("shapeOfTheEarth", code);
        switch (code) {
            case 1:
                info.extra_set("radius", spec.get_double("radius", a));
                break;
            case 3:
                info.extra_set("earthMajorAxis", spec.get_double("semi_major_axis", a) / 1000.);
                info.extra_set("earthMinorAxis", spec.get_double("semi_minor_axis", b) / 1000.);
                break;
            case 7:
                info.extra_set("earthMajorAxis", spec.get_double("semi_major_axis", a));
                info.extra_set("earthMinorAxis", spec.get_double("semi_minor_axis", b));
                break;
            default:
                break;
        }
    }
    else if (spec.has("radius")) {
        info.extra_set("shapeOfTheEarth", 1L);
        info.extra_set("radius", spec.get_double("radius"));
    }
    else if (spec.has("semi_major_axis") && spec.has("semi_minor_axis")) {
        info.extra_set("shapeOfTheEarth", 7L);
        info.extra_set("earthMajorAxis", spec.get_double("semi_major_axis"));
        info.extra_set("earthMinorAxis", spec.get_double("semi_minor_axis"));
    }
}


void Shape::fillJob(api::MIRJob& job, const Projection& proj) const {
    // MIRJob encoding of user-provided shape
    std::ostringstream shape;

    // shape given by radius or semi-major/minor axis
    if (const auto& spec = proj.spec(); provided) {
        shape << ";shapeOfTheEarth=" << code;
        switch (code) {
            case 1:
                shape << ";radius", spec.get_double("radius", a);
                break;
            case 3:
                shape << ";earthMajorAxis=" << spec.get_double("semi_major_axis", a) / 1000.;
                shape << ";earthMinorAxis=" << spec.get_double("semi_minor_axis", b) / 1000.;
                break;
            case 7:
                shape << ";earthMajorAxis=" << spec.get_double("semi_major_axis", a);
                shape << ";earthMinorAxis=" << spec.get_double("semi_minor_axis", b);
                break;
            default:
                break;
        }
    }
    else if (spec.has("radius")) {
        shape << ";shapeOfTheEarth=" << 1L;
        shape << ";radius", spec.get_double("radius");
    }
    else if (spec.has("semi_major_axis") && spec.has("semi_minor_axis")) {
        shape << ";shapeOfTheEarth=" << 7L;
        shape << ";earthMajorAxis=" << spec.get_double("semi_major_axis");
        shape << ";earthMinorAxis=" << spec.get_double("semi_minor_axis");
    }

    std::string grid;
    ASSERT(job.get("grid", grid) && !grid.empty());

    job.set("grid", grid + shape.str());
}


}  // namespace mir::util

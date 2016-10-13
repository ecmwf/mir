/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/data/FieldInfo.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/StringTools.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace data {


namespace {


bool is_valid_field_info(const std::string& info) {
    const char * valid[] = {
        "scalar",

        "vector.1d.polar.angle",
        "vector.2d.polar.angle", "vector.2d.polar.radius",
        "vector.3d.polar.angle", "vector.3d.polar.radius", "vector.3d.polar.height",

        "vector.2d.cartesian.x", "vector.2d.cartesian.y",
        "vector.3d.cartesian.x", "vector.3d.cartesian.y", "vector.3d.cartesian.z",

        0,
    };

    size_t i = 0;
    while (valid[i]) {
        if (info == valid[i++]) {
            return true;
        }
    }
    return false;
}


}  // (anonymous namespace)


FieldInfo::FieldInfo(size_t dimension, FieldInfo::Component component) {
    set(dimension, component);
}


FieldInfo::FieldInfo(const param::MIRParametrisation& params) {

    // (defaults to scalar)
    size_t dimension = 1;
    Component component = NONE;

    std::string info;
    if (params.get("FieldInfo", info)) {

        get(info, dimension, component);
        set(dimension, component);

        if (isAngle()) {

            bool degree    = true;
            bool symmetric = false;
            params.get("degree",    degree);
            params.get("symmetric", symmetric);

            component_ = degree?
                             (symmetric? CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC
                                       : CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC)
                           : (symmetric? CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC
                                       : CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC);

        }

    } else {

        set(dimension, component);

    }
}


FieldInfo::FieldInfo(const FieldInfo& other) {
    operator=(other);
}


FieldInfo& FieldInfo::operator=(const FieldInfo& other) {
    set(other.dimension_, other.component_);
    return *this;
}


mir::data::FieldInfo::operator std::string() const {
    if (isScalar()) {
        return "scalar";
    }

    std::string info = "vector";

    info += dimension_ == 1? ".1d"
          : dimension_ == 2? ".2d"
          : dimension_ == 3? ".3d"
          : "";

    info += component_ == CARTESIAN_X?        ".cartesian.x"
          : component_ == CARTESIAN_Y?        ".cartesian.y"
          : component_ == CARTESIAN_Z?        ".cartesian.z"
          : isAngle()?                        ".polar.angle"
          : component_ == CYLINDRICAL_RADIUS? ".polar.radius"
          : component_ == CYLINDRICAL_HEIGHT? ".polar.height"
          : "";

    // ensure this is a recognized FieldInfo
    if (!is_valid_field_info(info)) {
        throw eckit::SeriousBug("Fieldinfo: dimension/components combination: \"" + info + "\"", Here());
    }
    return info;
}


void FieldInfo::set(size_t dimension, Component component) {
    dimension_ = dimension;
    component_ = component;
    ASSERT(1 <= dimension_ && dimension_ <= 3);
    ASSERT((dimension_ == 1) || (1 <= component_ && component_ < ALL_COMPONENTS));
}


void FieldInfo::get(const std::string& info, size_t& dimension, FieldInfo::Component& component) {

    // ensure this is a recognized FieldInfo
    if (!is_valid_field_info(info)) {
        throw eckit::SeriousBug("Fieldinfo: unrecognized \"" + info + "\"", Here());
    }

    // (defaults to scalar)
    dimension = 1;
    component = NONE;

    // FIXME: improve on ugly parsing
    std::vector<std::string> split = eckit::StringTools::split(".", info);
    if (split.size() == 4 && split[0] == "vector") {

        size_t dim = split[1] == "1d"? 1 : split[1] == "2d"? 2 : split[1] == "3d"? 3 : 0;
        if (!dim) {
            return;
        }
        dimension = dim;

        const std::string comp = split[2] + "." + split[3];
        component = comp == "cartesian.x"?  CARTESIAN_X :
                    comp == "cartesian.y"?  CARTESIAN_Y :
                    comp == "cartesian.z"?  CARTESIAN_Z :
                    comp == "polar.angle"?  CYLINDRICAL_ANGLE  :
                    comp == "polar.radius"? CYLINDRICAL_RADIUS :
                    comp == "polar.height"? CYLINDRICAL_HEIGHT :
                                            NONE;
    }
}


}  // namespace data
}  // namespace mir

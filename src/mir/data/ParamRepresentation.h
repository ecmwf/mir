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


#ifndef mir_data_ParameterRepresentation_H
#define mir_data_ParameterRepresentation_H

#include <cstddef>


namespace mir {
namespace data {


/**
 * @brief ParamRepresentation describes how a parameter is represented in space
 */
struct ParamRepresentation {

    enum Component {
        NONE = 0,

        // Cartesian representations
        CARTESIAN_X = 1, CARTESIAN_Y, CARTESIAN_Z,

        // Cylindrical/polar representations
        // @note angles in degrees [0,360[/[-180,180] or radians [0,2π[/[-π,π]
        CYLINDRICAL_ANGLE = 4,
        CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC = 4, CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC,
        CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC,     CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC,
        ALL_CYLINDRICAL_ANGLES = 8,
        CYLINDRICAL_RADIUS = 9, CYLINDRICAL_HEIGHT,

        ALL_COMPONENTS = 11
    };

    ParamRepresentation(size_t dimension, Component component);

    size_t    dimension() const { return dimension_; }
    Component component() const { return component_; }

    void dimension(size_t dimension)    { dimension_ = dimension; }
    void component(Component component) { component_ = component; }

    bool isVector() const {
        return dimension_ > 1;
    }

    bool isScalar() const {
        return dimension_ == 1;
    }

    bool isAngle()  const {
        return (CYLINDRICAL_ANGLE <= component_ && component_ <= ALL_CYLINDRICAL_ANGLES);
    }

    bool isVectorInCartesianRepr() const {
        return isVector() && (CARTESIAN_X <= component_ && component_ <= CARTESIAN_Z);
    }

    bool isVectorInPolarRepr() const {
        return isVector() && (CYLINDRICAL_ANGLE <= component_ && component_ <= CYLINDRICAL_HEIGHT);
    }

    bool isAngleInDegreesAssymmetric() const { return component_ == CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC; }
    bool isAngleInDegreesSymmetric()   const { return component_ == CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC; }
    bool isAngleInRadiansAssymmetric() const { return component_ == CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC; }
    bool isAngleInRadiansSymmetric()   const { return component_ == CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC; }

private:

    size_t dimension_;
    Component component_;

};


}  // namespace data
}  // namespace mir


#endif

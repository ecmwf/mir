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


#ifndef mir_data_FieldInfo_H
#define mir_data_FieldInfo_H

#include <cstddef>
#include <string>


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace data {


/**
 * @brief FieldInfo describes a representation in space.
 */
class FieldInfo {
public:

    // -- Types

    enum Component {
        NONE = 0,

        // Cartesian representations
        CARTESIAN_X = 1,
        CARTESIAN_Y = 2,
        CARTESIAN_Z = 3,

        // Cylindrical/polar representations
        // @note angles in degrees [0,360[/[-180,180] or radians [0,2π[/[-π,π]
        CYLINDRICAL_ANGLE = 4,
        CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC = 4,
        CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC   = 5,
        CYLINDRICAL_ANGLE_RADIANS_ASSYMMETRIC = 6,
        CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC   = 7,
        ALL_CYLINDRICAL_ANGLES = 8,
        CYLINDRICAL_RADIUS =  9,
        CYLINDRICAL_HEIGHT = 10,

        ALL_COMPONENTS = 11
    };

    // -- Contructors

    explicit FieldInfo(size_t dimension=1, Component component=NONE);
    FieldInfo(const param::MIRParametrisation&);
    FieldInfo(const FieldInfo&);

    // -- Destructor

    ~FieldInfo() {}

    // -- Operators

    FieldInfo& operator=(const FieldInfo&);

    operator std::string() const;

    // -- Methods

    size_t    dimension() const { return dimension_; }
    Component component() const { return component_; }

    bool isScalar() const {
        return component_ == NONE;
    }

    bool isVector() const {
        return component_ != NONE;
    }

    bool isVectorInCartesianRepr() const {
        return isVector()
            && component_ >= CARTESIAN_X
            && component_ <= CARTESIAN_Z;
    }

    bool isVectorInPolarRepr() const {
        return isVector()
            && component_ >= CYLINDRICAL_ANGLE
            && component_ <= CYLINDRICAL_HEIGHT;
    }

    bool isAngle() const {
        return component_ >= CYLINDRICAL_ANGLE
            && component_ <= ALL_CYLINDRICAL_ANGLES;
    }

    bool isAngleInDegrees() const {
        return component_ == CYLINDRICAL_ANGLE_DEGREES_ASSYMMETRIC
            || component_ == CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC;
    }

    bool isAngleSymmetric() const {
        return component_ == CYLINDRICAL_ANGLE_DEGREES_SYMMETRIC
            || component_ == CYLINDRICAL_ANGLE_RADIANS_SYMMETRIC;
    }

private:

    // -- Members

    /// Parameter vector field dimension (for scalars, 1)
    size_t dimension_;

    /// Parameter vector field dimension component (for scalars, Component::NONE)
    Component component_;

    // -- Methods

    void set(size_t dimension, Component component);

    static void get(const std::string& info, size_t& dimension, FieldInfo::Component& component);

};


}  // namespace param
}  // namespace mir


#endif

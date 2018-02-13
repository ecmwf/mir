/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_MeshGeneratorParameters_h
#define mir_util_MeshGeneratorParameters_h

#include <string>
#include "atlas/meshgenerator.h"
#include "mir/util/Domain.h"


namespace eckit {
class MD5;
}
namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace util {


// Deriving from any eckit::Parametrisation should work
class MeshGeneratorParameters : public atlas::MeshGenerator::Parameters {
public:
    MeshGeneratorParameters();

    void hash(eckit::MD5&) const;
    void print(std::ostream&) const;
    std::string meshGenerator_;
    bool patchNorthPole_;
    bool patchSouthPole_;
    bool meshXYZField_;
    std::string file_;

private:

    friend std::ostream& operator<<(std::ostream& s, const MeshGeneratorParameters& p) {
        p.print(s);
        return s;
    }
};


}  // namespace util
}  // namespace mir


#endif


/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_MeshGeneratorParameters_h
#define mir_util_MeshGeneratorParameters_h

#include <iosfwd>
#include <string>
#include "mir/api/Atlas.h"


namespace eckit {
class MD5;
}
namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}


namespace mir {
namespace util {


class MeshGeneratorParameters : public atlas::MeshGenerator::Parameters {
public:
    MeshGeneratorParameters();
    MeshGeneratorParameters(const std::string& label, const param::MIRParametrisation&);

    void setOptions(const repres::Representation&);

    bool sameAs(const MeshGeneratorParameters&) const;
    void hash(eckit::Hash&) const;
    void print(std::ostream&) const;

    std::string meshGenerator_;
    bool meshCellCentres_;
    std::string fileLonLat_;
    std::string fileXY_;
    std::string fileXYZ_;

private:

    friend std::ostream& operator<<(std::ostream& s, const MeshGeneratorParameters& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif


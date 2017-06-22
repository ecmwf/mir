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
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   May 2016


#ifndef mir_method_MIRGrid_h
#define mir_method_MIRGrid_h

#include <vector>
#include "eckit/memory/NonCopyable.h"
#include "eckit/memory/ScopedPtr.h"
#include "atlas/array/Array.h"
#include "atlas/grid/Grid.h"
#include "atlas/meshgenerator.h"
#include "mir/util/Domain.h"


namespace eckit {
class MD5;
}
namespace atlas {
class Mesh;
}
namespace mir {
namespace method {
class MethodWeighted;
}
namespace repres {
class Representation;
}
}


namespace mir {
namespace util {


class MIRGrid {
public:

    // -- Types

    // Deriving from any eckit::Parametrisation should work
    class MeshGenParams: public atlas::MeshGenerator::Parameters {
    public:
        MeshGenParams();
    };

    // -- Contructors

    MIRGrid(const MIRGrid& other);

    explicit MIRGrid(const atlas::Grid& grid, const Domain&);

    // -- Operators

    MIRGrid& operator=(const MIRGrid& other);

    // -- Methods

    const Domain& domain() const;

    operator const atlas::Grid&() const;

    atlas::Mesh& mesh(const method::MethodWeighted&) const;
    const atlas::array::Array& coordsLonLat() const;
    const atlas::array::Array& coordsXYZ() const;

    void hash(eckit::MD5&) const;

    size_t size() const;


private:

    // -- Members

    const Domain domain_;
    const atlas::Grid grid_;
    mutable atlas::Mesh* mesh_;
    mutable eckit::ScopedPtr< atlas::array::Array > coordsLonLat_;
    mutable eckit::ScopedPtr< atlas::array::Array > coordsXYZ_;

};


}  // namespace util
}  // namespace mir


#endif


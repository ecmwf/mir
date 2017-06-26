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

#include <string>
#include <vector>
#include "eckit/memory/ScopedPtr.h"
#include "atlas/array/Array.h"
#include "atlas/grid/Grid.h"
#include "atlas/meshgenerator.h"
#include "mir/util/Domain.h"


namespace eckit {
class MD5;
}
namespace mir {
namespace util {
class MIRStatistics;
}
}


namespace mir {
namespace util {


class MIRGrid {
public:

    // -- Types

    // Deriving from any eckit::Parametrisation should work
    class MeshGenParams : public atlas::MeshGenerator::Parameters {
    public:
        MeshGenParams();
        std::string meshGenerator_;
    };

    // -- Contructors

    explicit MIRGrid(const atlas::Grid&, const Domain&);
    explicit MIRGrid(const atlas::Grid&, const Domain&, util::MIRStatistics&, const MeshGenParams&);

    MIRGrid(const MIRGrid&);

    // -- Methods

    operator const atlas::Grid&() const;
    const Domain& domain() const;
    atlas::Mesh& mesh() const;
    const atlas::array::Array& coordsXYZ() const;

    void hash(eckit::MD5&) const;
    size_t size() const;


private:

    // -- Methods

    atlas::Mesh generateMeshAndCache() const;

    // -- Members

    atlas::Grid grid_;
    Domain domain_;

    MeshGenParams meshGenParams_;
    util::MIRStatistics& statistics_;

    mutable atlas::Mesh mesh_;
    mutable eckit::ScopedPtr< atlas::array::Array > coordsXYZ_;

};


}  // namespace util
}  // namespace mir


#endif


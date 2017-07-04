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
        void hash(eckit::MD5&) const;
        std::string meshGenerator_;
        bool meshParallelEdgesConnectivity_;
        bool meshXYZField_;
        bool meshCellCentres_;
        bool dump_;
    };

    // -- Contructors

    explicit MIRGrid(const atlas::Grid&);

    MIRGrid(const MIRGrid&);

    // -- Methods

    operator const atlas::Grid&() const;
    const atlas::Mesh& mesh(util::MIRStatistics&, const MeshGenParams&) const;
    const atlas::Mesh& mesh() const;

    void hash(eckit::MD5&) const;
    size_t size() const;

private:

    // -- Members

    atlas::Grid grid_;
    mutable atlas::Mesh mesh_;
    mutable MeshGenParams meshGenParams_;

    // -- Methods

    atlas::Mesh generateMeshAndCache(util::MIRStatistics&, const MeshGenParams&) const;

};


}  // namespace util
}  // namespace mir


#endif


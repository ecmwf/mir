/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef mir_util_MIRGrid_h
#define mir_util_MIRGrid_h

#include <string>
#include <vector>
#include "mir/api/Atlas.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"


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

    // -- Contructors

    explicit MIRGrid(const atlas::Grid&);

    MIRGrid(const MIRGrid&);

    // -- Methods

    const atlas::Mesh& mesh(MIRStatistics&, const MeshGeneratorParameters&) const;
    const atlas::Mesh& mesh() const;
    double getMeshLongestElementDiagonal() const;

    void hash(eckit::MD5&) const;

private:

    // -- Members

    atlas::Grid grid_;
    mutable atlas::Mesh mesh_;
    mutable MeshGeneratorParameters meshGenParams_;

    // -- Methods

    atlas::Mesh generateMeshAndCache(MIRStatistics&, const MeshGeneratorParameters&) const;

};


}  // namespace util
}  // namespace mir


#endif


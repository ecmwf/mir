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


#include "mir/util/MeshGeneratorParameters.h"

#include <algorithm>

#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir::util {

MeshGeneratorParameters::MeshGeneratorParameters(const param::MIRParametrisation& param, const std::string& l) :
    meshCellCentres_(false),
    meshCellLongestDiagonal_(false),
    meshNodeLumpedMassMatrix_(false),
    meshNodeToCellConnectivity_(false) {
    set("3d", true);
    set("triangulate", false);
    set("angle", 0.);
    set("force_include_north_pole", false);
    set("force_include_south_pole", false);

    auto label(l.empty() ? "" : l + "-");

    const param::MIRParametrisation& user = param.userParametrisation();
    user.get(label + "mesh-generator", meshGenerator_);
    user.get(label + "mesh-cell-centres", meshCellCentres_);
    user.get(label + "mesh-cell-longest-diagonal", meshCellLongestDiagonal_);
    user.get(label + "mesh-node-lumped-mass-matrix", meshNodeLumpedMassMatrix_);
    user.get(label + "mesh-node-to-cell-connectivity", meshNodeToCellConnectivity_);
    user.get(label + "mesh-file-ll", fileLonLat_);
    user.get(label + "mesh-file-xy", fileXY_);
    user.get(label + "mesh-file-xyz", fileXYZ_);

    for (const auto& k : {"triangulate", "force_include_north_pole", "force_include_south_pole"}) {
        auto key   = label + "mesh-generator-" + std::string(k);
        auto value = false;
        std::replace(key.begin(), key.end(), '_', '-');

        user.get(key, value);
        set(k, value);
    }

    bool three_dimensional = true;
    user.get(label + "mesh-generator-three-dimensional", three_dimensional);
    set("3d", three_dimensional);

    double angle = getDouble("angle");
    if (user.get(label + "mesh-generator-angle", angle)) {
        set("angle", angle);
    }
}

bool MeshGeneratorParameters::sameAs(const MeshGeneratorParameters& other) const {
    eckit::MD5 a;
    eckit::MD5 b;
    hash(a);
    other.hash(b);

    return a.digest() == b.digest();
}

void MeshGeneratorParameters::hash(eckit::Hash& hash) const {
    hash << meshGenerator_;
    hash << meshCellCentres_;
    hash << meshCellLongestDiagonal_;
    hash << meshNodeLumpedMassMatrix_;
    hash << meshNodeToCellConnectivity_;
    atlas::MeshGenerator::Parameters::hash(hash);
}

void MeshGeneratorParameters::print(std::ostream& s) const {
    s << "MeshGeneratorParameters["
      << "meshGenerator=" << meshGenerator_ << ",meshCellCentres=" << meshCellCentres_
      << ",meshCellLongestDiagonal=" << meshCellLongestDiagonal_
      << ",meshNodeLumpedMassMatrix=" << meshNodeLumpedMassMatrix_
      << ",meshNodeToCellConnectivity=" << meshNodeToCellConnectivity_ << ",";
    atlas::MeshGenerator::Parameters::print(s);
    s << "]";
}

}  // namespace mir::util

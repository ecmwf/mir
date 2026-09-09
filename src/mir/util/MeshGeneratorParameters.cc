// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/MeshGeneratorParameters.h"

#include <algorithm>

#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir::util {

MeshGeneratorParameters::MeshGeneratorParameters() :
    meshCellCentres_(false), meshCellLongestDiagonal_(false), meshNodeToCellConnectivity_(false) {
    set("3d", true);
    set("triangulate", false);
    set("angle", 0.);
    set("force_include_north_pole", false);
    set("force_include_south_pole", false);
}

MeshGeneratorParameters::MeshGeneratorParameters(const param::MIRParametrisation& param) : MeshGeneratorParameters() {
    const param::MIRParametrisation& user = param.userParametrisation();
    user.get("mesh-generator", meshGenerator_);
    user.get("mesh-cell-centres", meshCellCentres_);
    user.get("mesh-cell-longest-diagonal", meshCellLongestDiagonal_);
    user.get("mesh-node-to-cell-connectivity", meshNodeToCellConnectivity_);
    user.get("mesh-file-ll", fileLonLat_);
    user.get("mesh-file-xy", fileXY_);
    user.get("mesh-file-xyz", fileXYZ_);

    for (const auto* k : {"triangulate", "force_include_north_pole", "force_include_south_pole"}) {
        auto key   = std::string("mesh-generator-") + k;
        auto value = false;
        std::replace(key.begin(), key.end(), '_', '-');

        user.get(key, value);
        set(k, value);
    }

    bool three_dimensional = true;
    user.get("mesh-generator-three-dimensional", three_dimensional);
    set("3d", three_dimensional);

    if (std::string extension_grid; user.get("mesh-generator-extension-grid", extension_grid)) {
        set("extension_grid", extension_grid);
    };

    if (auto angle = getDouble("angle"); user.get("mesh-generator-angle", angle)) {
        set("angle", angle);
    }
}

bool MeshGeneratorParameters::sameAs(const MeshGeneratorParameters& other) const {
    return (eckit::MD5{} << *this).digest() == (eckit::MD5{} << other).digest();
}

void MeshGeneratorParameters::hash(eckit::Hash& hash) const {
    hash << meshGenerator_;
    hash << meshCellCentres_;
    hash << meshCellLongestDiagonal_;
    hash << meshNodeToCellConnectivity_;
    atlas::MeshGenerator::Parameters::hash(hash);
}

void MeshGeneratorParameters::print(std::ostream& s) const {
    s << "MeshGeneratorParameters["
      << "meshGenerator=" << meshGenerator_ << ",meshCellCentres=" << meshCellCentres_
      << ",meshCellLongestDiagonal=" << meshCellLongestDiagonal_
      << ",meshNodeToCellConnectivity=" << meshNodeToCellConnectivity_ << ",";
    atlas::MeshGenerator::Parameters::print(s);
    s << "]";
}

}  // namespace mir::util

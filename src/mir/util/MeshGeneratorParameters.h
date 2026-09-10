// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>

#include "mir/util/Atlas.h"


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::util {


class MeshGeneratorParameters : public atlas::MeshGenerator::Parameters {
public:
    // -- Constructors

    MeshGeneratorParameters();
    explicit MeshGeneratorParameters(const param::MIRParametrisation&);

    MeshGeneratorParameters(const MeshGeneratorParameters&) = default;
    MeshGeneratorParameters(MeshGeneratorParameters&&)      = default;

    // -- Destructor

    ~MeshGeneratorParameters() override = default;

    // -- Operators

    MeshGeneratorParameters& operator=(const MeshGeneratorParameters&) = default;
    MeshGeneratorParameters& operator=(MeshGeneratorParameters&&)      = default;

    // -- Members

    std::string meshGenerator_;
    std::string fileLonLat_;
    std::string fileXY_;
    std::string fileXYZ_;
    bool meshCellCentres_;
    bool meshCellLongestDiagonal_;
    bool meshNodeToCellConnectivity_;

    // -- Methods

    bool sameAs(const MeshGeneratorParameters&) const;
    void hash(eckit::Hash&) const override;
    void print(std::ostream&) const override;

private:
    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MeshGeneratorParameters& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util

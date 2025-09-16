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

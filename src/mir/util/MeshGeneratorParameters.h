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
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    MeshGeneratorParameters();
    MeshGeneratorParameters(const param::MIRParametrisation&, const std::string& label = "");
    MeshGeneratorParameters(const MeshGeneratorParameters&) = default;

    // -- Destructor

    ~MeshGeneratorParameters() override = default;

    // -- Convertors
    // None

    // -- Operators

    MeshGeneratorParameters& operator=(const MeshGeneratorParameters&) = default;

    // -- Members

    std::string meshGenerator_;
    std::string fileLonLat_;
    std::string fileXY_;
    std::string fileXYZ_;
    bool meshCellCentres_;
    bool meshCellLongestDiagonal_;
    bool meshNodeLumpedMassMatrix_;
    bool meshNodeToCellConnectivity_;

    // -- Methods

    bool sameAs(const MeshGeneratorParameters&) const;
    void hash(eckit::Hash&) const override;
    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MeshGeneratorParameters& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util

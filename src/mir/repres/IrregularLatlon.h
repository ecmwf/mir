// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/repres/Gridded.h"


namespace mir::repres {


class IrregularLatlon : public Gridded {
public:
    // -- Exceptions
    // None

    // -- Constructors

    IrregularLatlon(const param::MIRParametrisation&);

    // -- Destructor

    ~IrregularLatlon() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

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

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    IrregularLatlon();

    // -- Members

    double south_;
    double north_;
    double south_north_;

    double west_;
    double east_;
    double west_east_;

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods
    // None

    // -- Overridden methods

    size_t numberOfPoints() const override;
    bool getLongestElementDiagonal(double&) const override;
    void validate(const MIRValuesVector&) const override;

    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    void makeName(std::ostream&) const override;
    bool sameAs(const Representation&) const override;
    atlas::Grid atlasGrid() const override;
    util::Domain domain() const override;
    Iterator* iterator() const override;
    bool isPeriodicWestEast() const override;
    bool includesNorthPole() const override;
    bool includesSouthPole() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::repres

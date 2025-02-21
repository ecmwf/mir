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

#include "eckit/geo/grid/ORCA.h"

#include "mir/repres/Gridded.h"


namespace mir::repres {


class ORCA : public Gridded {
public:
    // -- Types

    using uid_t = eckit::geo::Grid::uid_t;

    // -- Constructors

    explicit ORCA(const uid_t& uid);
    explicit ORCA(const param::MIRParametrisation&);

    ORCA(const ORCA&) = delete;
    ORCA(ORCA&&)      = delete;

    // -- Destructor

    ~ORCA() override = default;

    // -- Operators

    ORCA& operator=(const ORCA&) = delete;
    ORCA& operator=(ORCA&&)      = delete;

private:
    // -- Members

    eckit::geo::grid::ORCA grid_;

    // -- Overridden methods

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    bool sameAs(const Representation&) const override;
    void validate(const MIRValuesVector& values) const override;

    ::atlas::Grid atlasGrid() const override;

    size_t numberOfPoints() const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    Iterator* iterator() const override;

#if 0
    // FUNKY
    
    const ::atlas::Grid::Spec spec_;
    mutable ::atlas::Grid grid_;
    
    bool sameAs(const Representation&) const override;
    void makeName(std::ostream&) const override;
    
    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;
    void fillJob(api::MIRJob&) const override;
    
    size_t numberOfPoints() const override;
    
    void print(std::ostream&) const override;
#endif
};


}  // namespace mir::repres

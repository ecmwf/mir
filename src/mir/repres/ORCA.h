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

#include <memory>

#include "mir/repres/Gridded.h"


namespace eckit::geo {
class Spec;
namespace grid {
class ORCA;
}
}  // namespace eckit::geo


namespace mir::param {
class MIRParametrisation;
}


namespace mir::repres {


class ORCA final : public Gridded {
public:
    // -- Constructors

    explicit ORCA(const std::string& uid);
    explicit ORCA(const param::MIRParametrisation&);

    explicit ORCA(eckit::geo::Spec*);

    ORCA(const ORCA&) = delete;
    ORCA(ORCA&&)      = delete;

    // -- Destructor

    ~ORCA() override = default;

    // -- Operators

    ORCA& operator=(const ORCA&) = delete;
    ORCA& operator=(ORCA&&)      = delete;

private:
    // -- Members

    mutable std::unique_ptr<eckit::geo::grid::ORCA> grid_;

    std::unique_ptr<eckit::geo::Spec> spec_user_;
    std::unique_ptr<eckit::geo::Spec> spec_;

    // -- Methods

    eckit::geo::grid::ORCA& grid() const;

    // -- Overridden methods

    // from Representation
    bool sameAs(const Representation&) const override;
    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;

    Iterator* iterator() const override;
    size_t numberOfPoints() const override;
    void validate(const MIRValuesVector&) const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    void print(std::ostream&) const override;
};


}  // namespace mir::repres

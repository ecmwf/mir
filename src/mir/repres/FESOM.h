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
class FESOM;
}
}  // namespace eckit::geo


namespace mir::param {
class MIRParametrisation;
}


namespace mir::repres {


class FESOM final : public Gridded {
public:
    // -- Constructors

    explicit FESOM(const std::string& uid);
    explicit FESOM(const param::MIRParametrisation&);

    explicit FESOM(eckit::geo::Spec*);

    FESOM(const FESOM&) = delete;
    FESOM(FESOM&&)      = delete;

    // -- Destructor

    ~FESOM() override = default;

    // -- Operators

    FESOM& operator=(const FESOM&) = delete;
    FESOM& operator=(FESOM&&)      = delete;

private:
    // -- Members

    mutable std::unique_ptr<eckit::geo::grid::FESOM> grid_;

    std::unique_ptr<eckit::geo::Spec> spec_user_;
    std::unique_ptr<eckit::geo::Spec> spec_;

    // -- Methods

    eckit::geo::grid::FESOM& grid() const;

    // -- Overridden methods

    // from Representation
    bool sameAs(const Representation&) const override;
    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillJob(api::MIRJob&) const override;

    Iterator* iterator() const override;
    size_t numberOfPoints() const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    void print(std::ostream&) const override;
};


}  // namespace mir::repres

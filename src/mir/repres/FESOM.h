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

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Gridded.h"
#include "mir/util/Grib.h"


namespace eckit::geo {
class Grid;
class Spec;
}  // namespace eckit::geo


namespace mir::repres {


class FESOM final : public Gridded {
public:
    // -- Constructors

    explicit FESOM(const std::string& uid);
    explicit FESOM(const param::MIRParametrisation&);

    FESOM(const FESOM&) = delete;
    FESOM(FESOM&&)      = delete;

    // -- Destructor

    ~FESOM() override = default;

    // -- Operators

    FESOM& operator=(const FESOM&) = delete;
    FESOM& operator=(FESOM&&)      = delete;

private:
    // -- Members

    mutable std::unique_ptr<eckit::geo::Grid> grid_;
    std::unique_ptr<eckit::geo::Spec> spec_;

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

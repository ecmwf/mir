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

#include <string>
#include <utility>

#include "mir/repres/Gridded.h"


namespace eckit::geo {
class Grid;
}


namespace mir::repres {


class HEALPix final : public Gridded {
public:
    // -- Types

    using points_type = std::pair<std::vector<double>, std::vector<double>>;

    // -- Constructors

    explicit HEALPix(size_t Nside, const std::string& order = "ring");
    explicit HEALPix(const param::MIRParametrisation&);

    // -- Methods

    size_t Nside() const { return Nside_; }

private:
    // -- Members

    size_t Nside_;
    std::string order_;

    mutable std::unique_ptr<eckit::geo::Grid> grid_;
    mutable points_type points_;

    // -- Methods

    std::string name() const;
    eckit::geo::Grid& grid() const;
    points_type& to_latlons() const;

    // -- Overridden methods

    bool sameAs(const Representation& other) const override {
        const auto* o = dynamic_cast<const HEALPix*>(&other);
        return (o != nullptr) && Nside_ == o->Nside_ && order_ == o->order_;
    }

    void makeName(std::ostream&) const override;

    void fillGrib(grib_info&) const override;
    void fillMeshGen(util::MeshGeneratorParameters&) const override;
    void fillJob(api::MIRJob&) const override;

    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    void validate(const MIRValuesVector&) const override;
    size_t numberOfPoints() const override;

    Iterator* iterator() const override;

    bool includesNorthPole() const override { return true; }
    bool includesSouthPole() const override { return true; }
    bool isPeriodicWestEast() const override { return true; }

    std::vector<util::GridBox> gridBoxes() const override;
    ::atlas::Grid atlasGrid() const override;
};


}  // namespace mir::repres

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


#include "mir/action/interpolate/Gridded2GridSpec.h"

#include <ostream>

#include "eckit/geo/grid/ReducedGaussian.h"
#include "eckit/geo/grid/RegularGaussian.h"
#include "eckit/geo/grid/RegularLL.h"
#include "eckit/geo/spec/Custom.h"
#include "eckit/parser/YAMLParser.h"

#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"
#include "mir/util/EckitGeo.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


class GridSpecParametrisation final : public param::MIRParametrisation {
public:
    explicit GridSpecParametrisation(const eckit::geo::Grid& grid) :
        custom_(eckit::geo::spec::Custom::make_from_value(eckit::YAMLParser::decodeString(grid.spec_str()))) {
        ASSERT(custom_);

        ASSERT(!custom_->has_custom("area"));        // FIXME current limitation
        ASSERT(!custom_->has_custom("projection"));  // FIXME current limitation(includes rotation)

        const auto type{grid.type()};

        if (type == "regular-ll") {
            const auto& ll = dynamic_cast<const eckit::geo::grid::RegularLL&>(grid);
            custom_->set("gridType", "regular_ll");

            if (!has("west_east_increment") && !has("south_north_increment")) {
                custom_->set("west_east_increment", ll.dlon());
                custom_->set("south_north_increment", ll.dlat());
            }

            if (!has("Ni") && !has("Nj")) {
                custom_->set("Ni", ll.nlon());
                custom_->set("Nj", ll.nlat());
            }
        }

        else if (type == "regular-gg") {
            const auto& gg = dynamic_cast<const eckit::geo::grid::RegularGaussian&>(grid);
            custom_->set("gridType", "regular_gg");

            if (!has("N")) {
                custom_->set("N", gg.N());
            }
        }

        else if (type == "reduced-gg") {
            const auto& gg = dynamic_cast<const eckit::geo::grid::ReducedGaussian&>(grid);
            custom_->set("gridType", "reduced_gg");

            if (!has("N")) {
                custom_->set("N", gg.N());
            }

            if (!has("pl")) {
                custom_->set("pl", gg.pl());
            }

            if (!has("Nj")) {
                custom_->set("Nj", gg.nj());
            }
        }
        else {
            throw exception::UserError("GridSpecParametrisation: unsupported type: '" + type + "'");
        }

        if (!has("north") && !has("west") && !has("south") && !has("east")) {
            const auto& bbox = grid.boundingBox();

            custom_->set("north", bbox.north);
            custom_->set("west", bbox.west);
            custom_->set("south", bbox.south);
            custom_->set("east", bbox.east);
        }

        ASSERT(has("gridType"));
    }

private:
    bool has(const std::string& name) const override { return custom_->has(name); }

    bool get(const std::string& name, std::string& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, bool& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, int& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, long& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, float& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, double& v) const override { return custom_->get(name, v); }

    bool get(const std::string& name, std::vector<int>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<long>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<float>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<double>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<std::string>& v) const override { return custom_->get(name, v); }

    bool get(const std::string& name, size_t& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<size_t>& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, long long& v) const override { return custom_->get(name, v); }
    bool get(const std::string& name, std::vector<long long>& v) const override { return custom_->get(name, v); }

    void print(std::ostream& out) const override { out << *custom_; }

    std::unique_ptr<eckit::geo::spec::Custom> custom_;
};


Gridded2GridSpec::Gridded2GridSpec(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    // assign grid
    std::string gridspec;
    ASSERT(key::grid::Grid::get("grid", gridspec, param));

    grid_.reset(eckit::geo::GridFactory::make_from_string(gridspec));
    ASSERT(grid_);

    // assign compatible parametrisation
    param_.reset(new GridSpecParametrisation(*grid_));
    ASSERT(param_);
}


bool Gridded2GridSpec::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2GridSpec*>(&other);
    return (o != nullptr) && (grid_ == o->grid_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2GridSpec::print(std::ostream& out) const {
    out << "Gridded2GridSpec[gridspec=" << grid_->spec_str() << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const char* Gridded2GridSpec::name() const {
    return "Gridded2GridSpec";
}


const repres::Representation* Gridded2GridSpec::outputRepresentation() const {
    return repres::RepresentationFactory::build(*param_);
}


static const ActionBuilder<Gridded2GridSpec> grid2grid("interpolate.grid2gridspec");


}  // namespace mir::action::interpolate

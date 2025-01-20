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


#include <memory>
#include <ostream>

#include "eckit/parser/YAMLParser.h"

#include "mir/key/grid/Grid.h"
#include "mir/key/grid/GridPattern.h"
#include "mir/key/grid/NamedGrid.h"
#include "mir/util/EckitGeo.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


class GridSpec : public Grid {
public:
    GridSpec(const std::string& key) : Grid(key, "gridspec") {}

    const repres::Representation* representation() const override {
        std::cout << std::endl;
        NOTIMP;
    }

    const repres::Representation* representation(const util::Rotation&) const override {
        std::cout << std::endl;
        NOTIMP;
    }

    const repres::Representation* representation(const param::MIRParametrisation&) const override {
        std::cout << std::endl;
        NOTIMP;
    }

    size_t gaussianNumber() const override { return default_gaussian_number(); }

protected:
    void print(std::ostream& out) const override { out << "GridSpec[key=" << key_ << "]"; }
};


class GridSpecPattern : public GridPattern {
public:
    explicit GridSpecPattern(const std::string& name) : GridPattern(name) { auto x = name; }

private:
    void print(std::ostream& out) const override {
        auto x = pattern_;
        out << "GridSpecPattern[pattern=" << pattern_ << "]";
    }

    const Grid* make(const std::string& name) const override { return new GridSpec(name); }

    std::string canonical(const std::string& name, const param::MIRParametrisation&) const override {
        std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(name));
        return grid->spec_str();
    }
};


static const GridSpecPattern __pattern("^[{].*[}]$");


}  // namespace mir::key::grid

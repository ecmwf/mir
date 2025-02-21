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


#include "mir/repres/HEALPix.h"

#include <memory>
#include <ostream>

#include "eckit/geo/spec/Custom.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
// #include "mir/util/GridBox.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


namespace {


const RepresentationBuilder<HEALPix> __repres("healpix");


std::unique_ptr<eckit::geo::spec::Custom> spec_from_args(size_t Nside, HEALPix::Ordering ordering) {
    auto spec = std::make_unique<eckit::geo::spec::Custom>();
    spec->set("type", "HEALPix");
    spec->set("Nside", Nside);
    spec->set("ordering", ordering);

    return spec;
}


std::unique_ptr<eckit::geo::spec::Custom> spec_from_parametrisation(const param::MIRParametrisation& param) {
    long Nside = 0;
    param.get("Nside", Nside);

    std::string ordering;
    param.get("ordering", ordering);

    return spec_from_args(Nside,
                          ordering == "nested" ? HEALPix::Ordering::healpix_nested : HEALPix::Ordering::healpix_ring);
}


}  // namespace


HEALPix::HEALPix(size_t Nside, Ordering ordering) :
    Geo(*spec_from_args(Nside, ordering)), grid_(dynamic_cast<decltype(grid_)>(Geo::grid())) {}


HEALPix::HEALPix(const param::MIRParametrisation& param) :
    Geo(*spec_from_parametrisation(param)), grid_(dynamic_cast<decltype(grid_)>(Geo::grid())) {}


void HEALPix::makeName(std::ostream& out) const {
    out << "H" << std::to_string(grid_.Nside()) << (grid_.ordering() == Ordering::healpix_nested ? "_nested" : "");
}


void HEALPix::fillGrib(grib_info& info) const {
    info.grid.grid_type = GRIB_UTIL_GRID_SPEC_HEALPIX;
    info.grid.N         = static_cast<long>(grid_.Nside());

    info.grid.longitudeOfFirstGridPointInDegrees = 45.;

    info.extra_set("orderingConvention", grid_.ordering() == Ordering::healpix_nested ? "nested" : "ring");
}


void HEALPix::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }
}


::atlas::Grid HEALPix::atlasGrid() const {
    const auto* ordering = grid_.ordering() == Ordering::healpix_nested ? "nested" : "ring";
    return {::atlas::Grid::Spec("type", "healpix").set("N", grid_.Nside()).set("ordering", ordering)};
}


std::vector<util::GridBox> mir::repres::HEALPix::gridBoxes() const {
    NOTIMP;
}


}  // namespace mir::repres

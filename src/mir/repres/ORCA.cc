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


#include "mir/repres/ORCA.h"

#include <ostream>

#include "eckit/geo/spec/Custom.h"

#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


static const RepresentationBuilder<ORCA> __repres("orca");


ORCA::ORCA(const Grid::uid_t& uid) :
    Geo(*std::unique_ptr<eckit::geo::spec::Custom>(new eckit::geo::spec::Custom{{"uid", uid}})),
    grid_(dynamic_cast<decltype(grid_)>(Geo::grid())) {
    ASSERT(grid_.Grid::spec().get_string("type") == "orca");
}


ORCA::ORCA(const param::MIRParametrisation& param) :
    ORCA([&param]() {
        Grid::uid_t uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


void ORCA::makeName(std::ostream& out) const {
    out << grid_.name() << "_" << grid_.arrangement() << "_" << grid_.uid();
}


void ORCA::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    const auto name = grid_.name();
    info.extra_set("unstructuredGridType", name.c_str());

    const auto arrangement = grid_.arrangement();
    info.extra_set("unstructuredGridSubtype", arrangement.c_str());

    const auto uid = grid_.uid();
    info.extra_set("uuidOfHGrid", uid.c_str());
}


void ORCA::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "orca";
    }
}


::atlas::Grid ORCA::atlasGrid() const {
#if mir_HAVE_ATLAS
    return {::atlas::Grid::Spec("type", "ORCA").set("uid", grid_.uid())};
#else
    NOTIMP;
#endif
}


}  // namespace mir::repres

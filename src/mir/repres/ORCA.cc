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

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::repres {


static const RepresentationBuilder<ORCA> __repres("orca");


ORCA::ORCA(const uid_t& uid) : grid_(uid) {}


ORCA::ORCA(const param::MIRParametrisation& param) :
    ORCA([&param]() {
        std::string uid;
        ASSERT(param.get("uid", uid));
        return uid;
    }()) {}


void ORCA::json(eckit::JSON& j) const {
    grid_.Grid::spec().json(j);
}


void ORCA::print(std::ostream& out) const {
    out << "ORCA[spec=" << grid_.spec_str() << "]";
}


void ORCA::makeName(std::ostream& out) const {
    out << grid_.name() << "_" << grid_.arrangement() << "_" << grid_.uid();
}


void ORCA::fillGrib(grib_info& info) const {
    info.grid.grid_type        = GRIB_UTIL_GRID_SPEC_UNSTRUCTURED;
    info.packing.editionNumber = 2;

    const auto orca_name = grid_.name();
    info.extra_set("unstructuredGridType", orca_name.c_str());

    const auto orca_arrangement = grid_.arrangement();
    info.extra_set("unstructuredGridSubtype", orca_arrangement.c_str());

    const auto orca_uid = grid_.uid();
    info.extra_set("uuidOfHGrid", orca_uid.c_str());
}


void ORCA::fillJob(api::MIRJob& job) const {
    job.set("grid", grid_.spec_str());
}


void ORCA::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "orca";
    }
}


bool mir::repres::ORCA::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const ORCA*>(&other);
    return (o != nullptr) && grid_ == o->grid_;
}


void ORCA::validate(const MIRValuesVector& values) const {
    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("ORCA", values.size(), numberOfPoints());
}


::atlas::Grid ORCA::atlasGrid() const {
    return {::atlas::Grid::Spec("type", "ORCA").set("uid", grid_.uid())};
}


size_t ORCA::numberOfPoints() const {
    return grid_.size();
}


Iterator* ORCA::iterator() const {
    NOTIMP;
}


}  // namespace mir::repres

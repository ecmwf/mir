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


#include "mir/input/GridSpecInput.h"

#include "eckit/geo/Grid.h"

#include "mir/data/MIRField.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"
#include "mir/util/ValueMap.h"


namespace mir::input {


static const ArtificialInputBuilder<GridSpecInput> __artificial("gridspec");


GridSpecInput::GridSpecInput() : size_(0) {}


GridSpecInput::GridSpecInput(const std::string& gridspec, bool gridded) {
    util::ValueMap map;
    map["gridspec"]                       = gridspec;
    map[gridded ? "gridded" : "spectral"] = true;

    setAuxiliaryInformation(map);
}


void GridSpecInput::setAuxiliaryInformation(const util::ValueMap& map) {
    ArtificialInput::setAuxiliaryInformation(map);

    std::string gridspec;
    ASSERT(parametrisation().get("gridspec", gridspec));

    ASSERT(!parametrisation().has("spectral") || !parametrisation().has("gridded"));
    parametrisation().set(parametrisation().has("spectral") ? "spectral" : "gridded", true);

    auto* ptr = new param::GridSpecParametrisation(gridspec);
    ASSERT(ptr != nullptr);

    size_ = ptr->grid().size();
    ASSERT(size_ > 0);

    inputParametrisation(ptr);
}


data::MIRField GridSpecInput::field() const {
    ASSERT(dimensions() > 0);
    ASSERT(size_ > 0);

    data::MIRField field(repres::RepresentationFactory::build(parametrisation()));

    MIRValuesVector values(size_, 0.);
    for (size_t which = 0; which < dimensions(); ++which) {
        field.update(values, which);
    }

    return field;
}


}  // namespace mir::input

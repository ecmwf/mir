// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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


GridSpecInput::GridSpecInput(const std::string& gridspec, bool gridded) : size_(0) {
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

    for (size_t which = 0; which < dimensions(); ++which) {
        MIRValuesVector values(size_, 0.);
        field.update(values, which);
    }

    return field;
}


}  // namespace mir::input

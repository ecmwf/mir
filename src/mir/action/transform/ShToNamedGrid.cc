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


#include "mir/action/transform/ShToNamedGrid.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/key/grid/Grid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToNamedGrid<Invtrans>::ShToNamedGrid(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("grid", grid_));
}


template <class Invtrans>
bool ShToNamedGrid<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToNamedGrid*>(&other);
    return (o != nullptr) && (grid_ == o->grid_);
}


template <class Invtrans>
void ShToNamedGrid<Invtrans>::print(std::ostream& out) const {
    out << "ShToNamedGrid[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",grid=" << grid_ << "]";
}


template <class Invtrans>
void ShToNamedGrid<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                      const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToNamedGrid<Invtrans>::name() const {
    return "ShToNamedGrid";
}


template <class Invtrans>
const repres::Representation* ShToNamedGrid<Invtrans>::outputRepresentation() const {
    const auto& ng = key::grid::Grid::lookup(grid_, parametrisation());
    return ng.representation();
}


static const ActionBuilder<ShToNamedGrid<InvtransScalar> > __action1("transform.sh-scalar-to-namedgrid");
static const ActionBuilder<ShToNamedGrid<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-namedgrid");


}  // namespace mir::action::transform

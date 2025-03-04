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


#include "mir/action/transform/ShToTypedGrid.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/key/grid/Grid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToTypedGrid<Invtrans>::ShToTypedGrid(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("grid", grid_));
}


template <class Invtrans>
bool ShToTypedGrid<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToTypedGrid*>(&other);
    return (o != nullptr) && (grid_ == o->grid_);
}


template <class Invtrans>
void ShToTypedGrid<Invtrans>::print(std::ostream& out) const {
    out << "ShToTypedGrid[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",grid=" << grid_ << "]";
}


template <class Invtrans>
void ShToTypedGrid<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                      const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToTypedGrid<Invtrans>::name() const {
    return "ShToTypedGrid";
}


template <class Invtrans>
const repres::Representation* ShToTypedGrid<Invtrans>::outputRepresentation() const {
    const auto& ng = key::grid::Grid::lookup(grid_, parametrisation());
    return ng.representation();
}


static const ActionBuilder<ShToTypedGrid<InvtransScalar> > __action1("transform.sh-scalar-to-typedgrid");
static const ActionBuilder<ShToTypedGrid<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-typedgrid");


}  // namespace mir::action::transform

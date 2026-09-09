// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToGridSpec.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/key/grid/Grid.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToGridSpec<Invtrans>::ShToGridSpec(const param::MIRParametrisation& param) : ShToGridded(param) {
    // assign gridspec
    std::string gridspec;
    ASSERT(key::grid::Grid::get("grid", gridspec, param));

    // assign compatible parametrisation
    param_ = std::make_unique<param::GridSpecParametrisation>(eckit::geo::GridFactory::make_from_string(gridspec));
    ASSERT(param_);
}


template <class Invtrans>
bool ShToGridSpec<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToGridSpec*>(&other);
    return (o != nullptr) && (param_->spec() == o->param_->spec()) && ShToGridded::sameAs(other);
}


template <class Invtrans>
void ShToGridSpec<Invtrans>::print(std::ostream& out) const {
    out << "ShToGridSpec[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",gridspec=" << param_->spec() << "]";
}


template <class Invtrans>
void ShToGridSpec<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                     const param::MIRParametrisation& param) const {
    Invtrans::sh2grid(field, trans, param);
}


template <class Invtrans>
const char* ShToGridSpec<Invtrans>::ShToGridSpec::name() const {
    return "ShToGridSpec";
}


template <class Invtrans>
const repres::Representation* ShToGridSpec<Invtrans>::outputRepresentation() const {
    return repres::RepresentationFactory::build(*param_);
}


static const ActionBuilder<ShToGridSpec<InvtransScalar> > __action1("transform.sh-scalar-to-gridspec");
static const ActionBuilder<ShToGridSpec<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-gridspec");


}  // namespace mir::action::transform

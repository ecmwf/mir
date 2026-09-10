// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToReducedGGPLGiven.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToReducedGGPLGiven<Invtrans>::ShToReducedGGPLGiven(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("pl", pl_));
}


template <class Invtrans>
bool ShToReducedGGPLGiven<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToReducedGGPLGiven*>(&other);
    return (o != nullptr) && (pl_ == o->pl_);
}


template <class Invtrans>
void ShToReducedGGPLGiven<Invtrans>::print(std::ostream& out) const {
    out << "ShToReducedGGPLGiven[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",pl=" << pl_.size() << "]";
}


template <class Invtrans>
void ShToReducedGGPLGiven<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                             const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
bool ShToReducedGGPLGiven<Invtrans>::getGriddedTargetName(std::string& name) const {
    name = "N" + std::to_string(pl_.size());
    return true;
}


template <class Invtrans>
const char* ShToReducedGGPLGiven<Invtrans>::name() const {
    return "ShToReducedGGPLGiven";
}


template <class Invtrans>
const repres::Representation* ShToReducedGGPLGiven<Invtrans>::outputRepresentation() const {
    size_t N = pl_.size() / 2;
    return new repres::gauss::reduced::ReducedFromPL(N, pl_);
}


static const ActionBuilder<ShToReducedGGPLGiven<InvtransScalar> > __action1(
    "transform.sh-scalar-to-reduced-gg-pl-given");
static const ActionBuilder<ShToReducedGGPLGiven<InvtransVodTouv> > __action2(
    "transform.sh-vod-to-uv-reduced-gg-pl-given");


}  // namespace mir::action::transform

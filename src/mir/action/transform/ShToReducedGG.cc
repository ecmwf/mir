// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToReducedGG.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToReducedGG<Invtrans>::ShToReducedGG(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("reduced", N_));
}


template <class Invtrans>
bool ShToReducedGG<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToReducedGG*>(&other);
    return (o != nullptr) && (N_ == o->N_);
}


template <class Invtrans>
void ShToReducedGG<Invtrans>::print(std::ostream& out) const {
    out << "ShToReducedGG[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",N=" << N_ << "]";
}


template <class Invtrans>
void ShToReducedGG<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                      const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
bool ShToReducedGG<Invtrans>::getGriddedTargetName(std::string& name) const {
    name = "N" + std::to_string(N_);
    return true;
}


template <class Invtrans>
const char* ShToReducedGG<Invtrans>::name() const {
    return "ShToReducedGG";
}


template <class Invtrans>
const repres::Representation* ShToReducedGG<Invtrans>::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


static const ActionBuilder<ShToReducedGG<InvtransScalar> > __action1("transform.sh-scalar-to-reduced-gg");
static const ActionBuilder<ShToReducedGG<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-reduced-gg");


}  // namespace mir::action::transform

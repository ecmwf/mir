// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToRegularGG.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToRegularGG<Invtrans>::ShToRegularGG(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("regular", N_));
}


template <class Invtrans>
bool ShToRegularGG<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToRegularGG*>(&other);
    return (o != nullptr) && (N_ == o->N_);
}


template <class Invtrans>
void ShToRegularGG<Invtrans>::print(std::ostream& out) const {
    out << "ShToRegularGG[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",N=" << N_ << "]";
}


template <class Invtrans>
void ShToRegularGG<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                      const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
bool ShToRegularGG<Invtrans>::getGriddedTargetName(std::string& name) const {
    name = "F" + std::to_string(N_);
    return true;
}


template <class Invtrans>
const char* ShToRegularGG<Invtrans>::name() const {
    return "ShToRegularGG";
}


template <class Invtrans>
const repres::Representation* ShToRegularGG<Invtrans>::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


static const ActionBuilder<ShToRegularGG<InvtransScalar> > __action1("transform.sh-scalar-to-regular-gg");
static const ActionBuilder<ShToRegularGG<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-regular-gg");


}  // namespace mir::action::transform

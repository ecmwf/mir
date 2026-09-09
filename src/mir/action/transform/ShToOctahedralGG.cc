// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToOctahedralGG.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToOctahedralGG<Invtrans>::ShToOctahedralGG(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("octahedral", N_));
}


template <class Invtrans>
bool ShToOctahedralGG<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToOctahedralGG*>(&other);
    return (o != nullptr) && (N_ == o->N_);
}


template <class Invtrans>
void ShToOctahedralGG<Invtrans>::print(std::ostream& out) const {
    out << "ShToOctahedralGG[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",N=" << N_ << "]";
}


template <class Invtrans>
void ShToOctahedralGG<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                         const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
bool ShToOctahedralGG<Invtrans>::getGriddedTargetName(std::string& name) const {
    name = "O" + std::to_string(N_);
    return true;
}


template <class Invtrans>
const char* ShToOctahedralGG<Invtrans>::name() const {
    return "ShToOctahedralGG";
}


template <class Invtrans>
const repres::Representation* ShToOctahedralGG<Invtrans>::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


static const ActionBuilder<ShToOctahedralGG<InvtransScalar> > __action1("transform.sh-scalar-to-octahedral-gg");
static const ActionBuilder<ShToOctahedralGG<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-octahedral-gg");


}  // namespace mir::action::transform

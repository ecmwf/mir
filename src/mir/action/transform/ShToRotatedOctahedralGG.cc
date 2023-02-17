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


#include "mir/action/transform/ShToRotatedOctahedralGG.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedOctahedral.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToRotatedOctahedralGG<Invtrans>::ShToRotatedOctahedralGG(const param::MIRParametrisation& parametrisation) :
    ShToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("octahedral", N_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT_KEYWORD_ROTATION_SIZE(value.size());
    rotation_ = util::Rotation(value[0], value[1]);
}


template <class Invtrans>
bool ShToRotatedOctahedralGG<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToRotatedOctahedralGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && (rotation_ == o->rotation_) && ShToGridded::sameAs(other);
}


template <class Invtrans>
ShToRotatedOctahedralGG<Invtrans>::~ShToRotatedOctahedralGG() = default;


template <class Invtrans>
void ShToRotatedOctahedralGG<Invtrans>::print(std::ostream& out) const {
    out << "ShToRotatedOctahedralGG[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",N=" << N_ << ",rotation=" << rotation_ << "]";
}


template <class Invtrans>
void ShToRotatedOctahedralGG<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                                const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToRotatedOctahedralGG<Invtrans>::name() const {
    return "ShToRotatedOctahedralGG";
}


template <class Invtrans>
const repres::Representation* ShToRotatedOctahedralGG<Invtrans>::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedOctahedral(N_, rotation_);
}


static const ActionBuilder<ShToRotatedOctahedralGG<InvtransScalar> > __action1(
    "transform.sh-scalar-to-rotated-octahedral-gg");
static const ActionBuilder<ShToRotatedOctahedralGG<InvtransVodTouv> > __action2(
    "transform.sh-vod-to-uv-rotated-octahedral-gg");


}  // namespace mir::action::transform

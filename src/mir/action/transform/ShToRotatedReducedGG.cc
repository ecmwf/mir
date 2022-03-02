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


#include "mir/action/transform/ShToRotatedReducedGG.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedClassic.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace transform {


template <class Invtrans>
ShToRotatedReducedGG<Invtrans>::ShToRotatedReducedGG(const param::MIRParametrisation& parametrisation) :
    ShToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("reduced", N_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT_KEYWORD_ROTATION_SIZE(value.size());
    rotation_ = util::Rotation(value[0], value[1]);
}


template <class Invtrans>
ShToRotatedReducedGG<Invtrans>::~ShToRotatedReducedGG() = default;


template <class Invtrans>
bool ShToRotatedReducedGG<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToRotatedReducedGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && (rotation_ == o->rotation_) && ShToGridded::sameAs(other);
}


template <class Invtrans>
void ShToRotatedReducedGG<Invtrans>::print(std::ostream& out) const {
    out << "ShToRotatedReducedGG[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",N=" << N_ << ",rotation=" << rotation_ << "]";
}


template <class Invtrans>
void ShToRotatedReducedGG<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                             const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToRotatedReducedGG<Invtrans>::name() const {
    return "ShToRotatedReducedGG";
}


template <class Invtrans>
const repres::Representation* ShToRotatedReducedGG<Invtrans>::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedClassic(N_, rotation_);
}


static const ActionBuilder<ShToRotatedReducedGG<InvtransScalar> > __action1(
    "transform.sh-scalar-to-rotated-reduced-gg");
static const ActionBuilder<ShToRotatedReducedGG<InvtransVodTouv> > __action2(
    "transform.sh-vod-to-uv-rotated-reduced-gg");


}  // namespace transform
}  // namespace action
}  // namespace mir

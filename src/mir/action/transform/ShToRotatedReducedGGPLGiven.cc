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


#include "mir/action/transform/ShToRotatedReducedGGPLGiven.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedFromPL.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToRotatedReducedGGPLGiven<Invtrans>::ShToRotatedReducedGGPLGiven(const param::MIRParametrisation& param) :
    ShToGridded(param) {
    ASSERT(parametrisation().userParametrisation().get("pl", pl_));

    std::vector<double> value;
    ASSERT(parametrisation().userParametrisation().get("rotation", value));
    ASSERT_KEYWORD_ROTATION_SIZE(value.size());
    rotation_ = util::Rotation(value[0], value[1]);
}


template <class Invtrans>
bool ShToRotatedReducedGGPLGiven<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToRotatedReducedGGPLGiven*>(&other);
    return (o != nullptr) && (pl_ == o->pl_) && (rotation_ == o->rotation_) && ShToGridded::sameAs(other);
}


template <class Invtrans>
void ShToRotatedReducedGGPLGiven<Invtrans>::print(std::ostream& out) const {
    out << "ShToRotatedReducedGGPLGiven[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",pl=" << pl_.size() << ",rotation=" << rotation_ << "]";
}


template <class Invtrans>
void ShToRotatedReducedGGPLGiven<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                                    const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToRotatedReducedGGPLGiven<Invtrans>::name() const {
    return "ShToRotatedReducedGGPLGiven";
}


template <class Invtrans>
const repres::Representation* ShToRotatedReducedGGPLGiven<Invtrans>::outputRepresentation() const {
    size_t N = pl_.size() / 2;
    return new repres::gauss::reduced::RotatedFromPL(N, pl_, rotation_);
}


static const ActionBuilder<ShToRotatedReducedGGPLGiven<InvtransScalar> > __action1(
    "transform.sh-scalar-to-rotated-reduced-gg-pl-given");
static const ActionBuilder<ShToRotatedReducedGGPLGiven<InvtransVodTouv> > __action2(
    "transform.sh-vod-to-uv-rotated-reduced-gg-pl-given");


}  // namespace mir::action::transform

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/action/transform/ShToRotatedReducedGGPLGiven.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedFromPL.h"


namespace mir {
namespace action {
namespace transform {


template<class Invtrans>
ShToRotatedReducedGGPLGiven<Invtrans>::ShToRotatedReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    ShToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT(value.size() == 2);
    rotation_ = util::Rotation(value[0], value[1]);
}


template<class Invtrans>
ShToRotatedReducedGGPLGiven<Invtrans>::~ShToRotatedReducedGGPLGiven() {
}


template<class Invtrans>
bool ShToRotatedReducedGGPLGiven<Invtrans>::sameAs(const Action& other) const {
    const ShToRotatedReducedGGPLGiven* o = dynamic_cast<const ShToRotatedReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_) && (rotation_ == o->rotation_) && ShToGridded::sameAs(other);
}


template<class Invtrans>
void ShToRotatedReducedGGPLGiven<Invtrans>::print(std::ostream& out) const {
    out << "ShToRotatedReducedGGPLGiven[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",pl=" << pl_.size()
        << ",rotation=" << rotation_
        << "]";
}


template<class Invtrans>
void ShToRotatedReducedGGPLGiven<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans) const {
    Invtrans::sh2grid(field, trans);
}


template<class Invtrans>
const char* ShToRotatedReducedGGPLGiven<Invtrans>::name() const {
    return "ShToRotatedReducedGGPLGiven";
}


template<class Invtrans>
const repres::Representation* ShToRotatedReducedGGPLGiven<Invtrans>::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedFromPL(pl_, rotation_);
}


namespace {
static ActionBuilder< ShToRotatedReducedGGPLGiven<InvtransScalar> > __action1("transform.sh-scalar-to-rotated-reduced-gg-pl-given");
static ActionBuilder< ShToRotatedReducedGGPLGiven<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-rotated-reduced-gg-pl-given");
}


}  // namespace transform
}  // namespace action
}  // namespace mir


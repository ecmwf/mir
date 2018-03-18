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


#include "mir/action/transform/ShToRotatedRegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RotatedGG.h"


namespace mir {
namespace action {
namespace transform {


template<class Invtrans>
ShToRotatedRegularGG<Invtrans>::ShToRotatedRegularGG(const param::MIRParametrisation& parametrisation):
    ShToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("regular", N_));

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("rotation", value));
    ASSERT(value.size() == 2);
    rotation_ = util::Rotation(value[0], value[1]);

    local(true);
}


template<class Invtrans>
ShToRotatedRegularGG<Invtrans>::~ShToRotatedRegularGG() {
}


template<class Invtrans>
bool ShToRotatedRegularGG<Invtrans>::sameAs(const Action& other) const {
    const ShToRotatedRegularGG* o = dynamic_cast<const ShToRotatedRegularGG*>(&other);
    return o && (N_ == o->N_) && (rotation_ == o->rotation_) && ShToGridded::sameAs(other);
}


template<class Invtrans>
void ShToRotatedRegularGG<Invtrans>::print(std::ostream& out) const {
    out << "ShToRotatedRegularGG[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",N=" << N_
        << ",rotation=" << rotation_
        << "]";
}


template<class Invtrans>
void ShToRotatedRegularGG<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans, const atlas::Grid& grid) const {
    Invtrans::sh2grid(field, trans, grid);
}


template<class Invtrans>
const char* ShToRotatedRegularGG<Invtrans>::name() const {
    return "ShToRotatedRegularGG";
}


template<class Invtrans>
const repres::Representation* ShToRotatedRegularGG<Invtrans>::outputRepresentation() const {
    return new repres::gauss::regular::RotatedGG(N_, rotation_);
}


namespace {
static ActionBuilder< ShToRotatedRegularGG<InvtransScalar> > __action1("transform.sh-scalar-to-rotated-regular-gg");
static ActionBuilder< ShToRotatedRegularGG<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-rotated-regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir


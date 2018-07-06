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


#include "mir/action/transform/ShToRegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"


namespace mir {
namespace action {
namespace transform {


template<class Invtrans>
ShToRegularLL<Invtrans>::ShToRegularLL(const param::MIRParametrisation &parametrisation):
    ShToGridded(parametrisation) {

    std::vector<double> value;

    if (parametrisation.userParametrisation().get("area", value)) {
        ASSERT(value.size() == 4);
        bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }

    ASSERT(parametrisation.userParametrisation().get("grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);
}


template<class Invtrans>
ShToRegularLL<Invtrans>::~ShToRegularLL() = default;


template<class Invtrans>
bool ShToRegularLL<Invtrans>::sameAs(const Action& other) const {
    auto o = dynamic_cast<const ShToRegularLL*>(&other);
    return o && (increments_ == o->increments_);
}


template<class Invtrans>
void ShToRegularLL<Invtrans>::print(std::ostream& out) const {
    out << "ShToRegularLL[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",bbox=" << bbox_
        << ",increments=" << increments_
        << "]";
}


template<class Invtrans>
void ShToRegularLL<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans) const {
    Invtrans::sh2grid(field, trans);
}


template<class Invtrans>
const char* ShToRegularLL<Invtrans>::name() const {
    return "ShToRegularLL";
}


template<class Invtrans>
const repres::Representation* ShToRegularLL<Invtrans>::outputRepresentation() const {

    util::BoundingBox bbox(bbox_);
    increments_.globaliseBoundingBox(bbox);

    return new repres::latlon::RegularLL(increments_, bbox);
}


namespace {
static ActionBuilder< ShToRegularLL<InvtransScalar> > __action1("transform.sh-scalar-to-regular-ll");
static ActionBuilder< ShToRegularLL<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-regular-ll");
}


}  // namespace transform
}  // namespace action
}  // namespace mir


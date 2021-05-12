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


#include "mir/action/transform/ShToRotatedRegularLL.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RotatedLL.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace transform {


template <class Invtrans>
ShToRotatedRegularLL<Invtrans>::ShToRotatedRegularLL(const param::MIRParametrisation& parametrisation) :
    ShToGridded(parametrisation) {

    std::vector<double> value;

    if (parametrisation.userParametrisation().get("area", value)) {
        ASSERT_KEYWORD_AREA_SIZE(value.size());
        bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }

    ASSERT(parametrisation.userParametrisation().get("grid", value));
    ASSERT_KEYWORD_GRID_SIZE(value.size());
    increments_ = util::Increments(value[0], value[1]);

    ASSERT(parametrisation.userParametrisation().get("rotation", value));
    ASSERT_KEYWORD_ROTATION_SIZE(value.size());
    rotation_ = util::Rotation(value[0], value[1]);
}


template <class Invtrans>
ShToRotatedRegularLL<Invtrans>::~ShToRotatedRegularLL() = default;


template <class Invtrans>
bool ShToRotatedRegularLL<Invtrans>::sameAs(const Action& other) const {
    auto o = dynamic_cast<const ShToRotatedRegularLL*>(&other);
    return (o != nullptr) && (increments_ == o->increments_) && (rotation_ == o->rotation_) &&
           ShToGridded::sameAs(other);
}


template <class Invtrans>
void ShToRotatedRegularLL<Invtrans>::print(std::ostream& out) const {
    out << "ShToRotatedRegularLL[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",bbox=" << bbox_ << ",increments=" << increments_ << ",rotation=" << rotation_ << "]";
}


template <class Invtrans>
void ShToRotatedRegularLL<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                             const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToRotatedRegularLL<Invtrans>::name() const {
    return "ShToRotatedRegularLL";
}


template <class Invtrans>
const repres::Representation* ShToRotatedRegularLL<Invtrans>::outputRepresentation() const {

    util::BoundingBox bbox(bbox_);
    const PointLatLon ref(bbox_.south(), bbox_.west());

    repres::latlon::LatLon::globaliseBoundingBox(bbox, increments_, ref);

    return new repres::latlon::RotatedLL(increments_, rotation_, bbox, ref);
}


static ActionBuilder<ShToRotatedRegularLL<InvtransScalar> > __action1("transform.sh-scalar-to-rotated-regular-ll");
static ActionBuilder<ShToRotatedRegularLL<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-rotated-regular-ll");


}  // namespace transform
}  // namespace action
}  // namespace mir

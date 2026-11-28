// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToRegularLL.h"

#include <ostream>

#include "eckit/log/JSON.h"

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/key/Area.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToRegularLL<Invtrans>::ShToRegularLL(const param::MIRParametrisation& param) : ShToGridded(param) {
    key::Area::get(parametrisation().userParametrisation(), bbox_);

    std::vector<double> value;

    ASSERT(parametrisation().userParametrisation().get("grid", value));
    ASSERT_KEYWORD_GRID_SIZE(value.size());
    increments_ = util::Increments(value[0], value[1]);
}


template <class Invtrans>
bool ShToRegularLL<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToRegularLL*>(&other);
    return (o != nullptr) && (increments_ == o->increments_) && (bbox_ == o->bbox_) && ShToGridded::sameAs(other);
}


template <class Invtrans>
void ShToRegularLL<Invtrans>::json(eckit::JSON& j) const {
    ShToGridded::json(j);
    Invtrans::json(j);
    j << "grid" << spec_value(increments_);
    j << "area" << spec_value(bbox_);
}


template <class Invtrans>
void ShToRegularLL<Invtrans>::print(std::ostream& out) const {
    out << "ShToRegularLL[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",bbox=" << bbox_ << ",increments=" << increments_ << "]";
}


template <class Invtrans>
void ShToRegularLL<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                      const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToRegularLL<Invtrans>::name() const {
    return "ShToRegularLL";
}


template <class Invtrans>
const repres::Representation* ShToRegularLL<Invtrans>::outputRepresentation() const {

    util::BoundingBox bbox(bbox_);
    const PointLonLat ref{bbox_.west(), bbox_.south()};

    repres::latlon::LatLon::globaliseBoundingBox(bbox, increments_, ref);

    return new repres::latlon::RegularLL(increments_, bbox, ref);
}


static const ActionBuilder<ShToRegularLL<InvtransScalar> > __action1("transform.sh-scalar-to-regular-ll");
static const ActionBuilder<ShToRegularLL<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-regular-ll");


}  // namespace mir::action::transform

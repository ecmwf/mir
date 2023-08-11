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


#include "mir/action/transform/ShToPoints.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToPoints<Invtrans>::ShToPoints(const param::MIRParametrisation& parametrisation) : ShToGridded(parametrisation) {
#if 0
    auto& user  = parametrisation_.userParametrisation();
    auto& field = parametrisation_.fieldParametrisation();

    ASSERT(user.has("latitudes") && field.get("latitudes", latitudes_));
    ASSERT(user.has("longitudes") && field.get("longitudes", longitudes_));

    ASSERT(latitudes_.size() == longitudes_.size());
#else
    ASSERT_MSG(parametrisation_.userParametrisation().get("latitudes", latitudes_) &&
                   parametrisation_.userParametrisation().get("longitudes", longitudes_) && !latitudes_.empty() &&
                   latitudes_.size() == longitudes_.size(),
               "ShToPoints: requires 'latitudes' and 'longitudes', non-empty and of the same size");
#endif
}


template <class Invtrans>
ShToPoints<Invtrans>::~ShToPoints() = default;


template <class Invtrans>
bool ShToPoints<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToPoints*>(&other);
    return (o != nullptr) && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_);
}


template <class Invtrans>
void ShToPoints<Invtrans>::print(std::ostream& out) const {
    out << "ShToPoints[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",points=" << latitudes_.size() << "]";
}


template <class Invtrans>
void ShToPoints<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                   const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToPoints<Invtrans>::name() const {
    return "ShToPoints";
}


template <class Invtrans>
const repres::Representation* ShToPoints<Invtrans>::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}


static const ActionBuilder<ShToPoints<InvtransScalar> > __action1("transform.sh-scalar-to-points");
static const ActionBuilder<ShToPoints<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-points");


}  // namespace mir::action::transform

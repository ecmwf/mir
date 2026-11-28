// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/transform/ShToPoints.h"

#include <ostream>

#include "eckit/geo/grid/Unstructured.h"
#include "eckit/log/JSON.h"

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir::action::transform {


template <class Invtrans>
ShToPoints<Invtrans>::ShToPoints(const param::MIRParametrisation& param) : ShToGridded(param) {
    ASSERT_MSG(parametrisation().userParametrisation().get("latitudes", latitudes_) &&
                   parametrisation().userParametrisation().get("longitudes", longitudes_) && !latitudes_.empty() &&
                   latitudes_.size() == longitudes_.size(),
               "ShToPoints: requires 'latitudes' and 'longitudes', non-empty and of the same size");
}


template <class Invtrans>
bool ShToPoints<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToPoints*>(&other);
    return (o != nullptr) && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_) &&
           ShToGridded::sameAs(other);
}


template <class Invtrans>
void ShToPoints<Invtrans>::json(eckit::JSON& j) const {
    ShToGridded::json(j);
    Invtrans::json(j);
    // the points, by their (unstructured) grid uid
    j << "uid" << eckit::geo::grid::Unstructured::uid_from_latlons(latitudes_, longitudes_);
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
    return new repres::UnstructuredGrid(latitudes_, longitudes_);
}


static const ActionBuilder<ShToPoints<InvtransScalar> > __action1("transform.sh-scalar-to-points");
static const ActionBuilder<ShToPoints<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-points");


}  // namespace mir::action::transform

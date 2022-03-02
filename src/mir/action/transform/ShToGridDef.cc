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


#include "mir/action/transform/ShToGridDef.h"

#include <ostream>

#include "mir/action/transform/InvtransScalar.h"
#include "mir/action/transform/InvtransVodTouv.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace transform {


template <class Invtrans>
ShToGridDef<Invtrans>::ShToGridDef(const param::MIRParametrisation& parametrisation) : ShToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("griddef", griddef_));
}


template <class Invtrans>
ShToGridDef<Invtrans>::~ShToGridDef() = default;


template <class Invtrans>
bool ShToGridDef<Invtrans>::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const ShToGridDef*>(&other);
    return (o != nullptr) && (griddef_ == o->griddef_);
}


template <class Invtrans>
void ShToGridDef<Invtrans>::print(std::ostream& out) const {
    out << "ShToGridDef[";
    ShToGridded::print(out);
    out << ",";
    Invtrans::print(out);
    out << ",griddef=.../" << eckit::PathName(griddef_).baseName() << "]";
}


template <class Invtrans>
void ShToGridDef<Invtrans>::sh2grid(data::MIRField& field, const ShToGridded::atlas_trans_t& trans,
                                    const param::MIRParametrisation& parametrisation) const {
    Invtrans::sh2grid(field, trans, parametrisation);
}


template <class Invtrans>
const char* ShToGridDef<Invtrans>::ShToGridDef::name() const {
    return "ShToGridDef";
}


template <class Invtrans>
const repres::Representation* ShToGridDef<Invtrans>::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


static const ActionBuilder<ShToGridDef<InvtransScalar> > __action1("transform.sh-scalar-to-griddef");
static const ActionBuilder<ShToGridDef<InvtransVodTouv> > __action2("transform.sh-vod-to-uv-griddef");


}  // namespace transform
}  // namespace action
}  // namespace mir

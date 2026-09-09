// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/gridbox/GridBoxMethod.h"

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"


namespace mir::method::gridbox {


bool GridBoxMethod::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const GridBoxMethod*>(&other);
    return (o != nullptr) && type() == o->type() && MethodWeighted::sameAs(*o);
}


void GridBoxMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5.add(type());
}


void GridBoxMethod::print(std::ostream& out) const {
    out << "GridBoxMethod[type=" << type() << ",";
    MethodWeighted::print(out);
    out << "]";
}


WeightMatrix::Check GridBoxMethod::validateMatrixWeights() const {
    return {true, true, false};
}


}  // namespace mir::method::gridbox

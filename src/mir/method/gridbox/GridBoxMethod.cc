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


#include "mir/method/gridbox/GridBoxMethod.h"

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"


namespace mir::method::gridbox {


bool GridBoxMethod::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const GridBoxMethod*>(&other);
    return (o != nullptr) && name() == o->name() && MethodWeighted::sameAs(*o);
}


void GridBoxMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5.add(name());
}


void GridBoxMethod::json(eckit::JSON& j) const {
    j.startObject();
    MethodWeighted::json(j);
    j.endObject();
}


void GridBoxMethod::print(std::ostream& out) const {
    out << "GridBoxMethod[name=" << name() << ",";
    MethodWeighted::print(out);
    out << "]";
}


bool GridBoxMethod::validateMatrixWeights() const {
    return false;
}


}  // namespace mir::method::gridbox

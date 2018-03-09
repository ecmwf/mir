/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/action/transform/CompressIfNotGlobal.h"

#include <iostream>
#include "mir/util/BoundingBox.h"


namespace mir {
namespace action {
namespace transform {


static CompressIfBuilder< CompressIfNotGlobal > __compressIf("not-global");


CompressIfNotGlobal::CompressIfNotGlobal(const param::MIRParametrisation& parametrisation) :
    CompressIf(parametrisation) {
}


bool CompressIfNotGlobal::operator()(const util::BoundingBox& cropping) const {
    static util::BoundingBox global;
    return cropping != global;
}


void CompressIfNotGlobal::print(std::ostream& out) const {
    out << "CompressIfNotGlobal[]";
}


}  // namespace transform
}  // namespace action
}  // namespace mir


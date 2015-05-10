/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date Apr 2015


#include <string>

#include "mir/util/PointSearch.h"

#include "mir/method/NearestNeighbourg.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {


NearestNeighbourg::NearestNeighbourg(const param::MIRParametrisation &param) :
    Nearest(param) {
}


NearestNeighbourg::~NearestNeighbourg() {
}

size_t NearestNeighbourg::nclosest() const {
    return 1;
}

const char *NearestNeighbourg::name() const {
    return  "nearest-neighbourg";
}

void NearestNeighbourg::hash( eckit::MD5& md5) const {
    Nearest::hash(md5);
}


void NearestNeighbourg::print(std::ostream &out) const {
    out << "NearestNeighbourg[epsilon=" << epsilon_ << "]";
}


namespace {
static MethodBuilder< NearestNeighbourg > __knearest1("nearest-neighbourg");
static MethodBuilder< NearestNeighbourg > __knearest2("nearest-neighbor"); // For the americans
static MethodBuilder< NearestNeighbourg > __knearest3("nn"); // For the lazy

}


}  // namespace method
}  // namespace mir


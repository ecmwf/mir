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
/// @date May 2015


#include "mir/method/NearestNeighbour.h"

#include "eckit/utils/MD5.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {


NearestNeighbour::NearestNeighbour(const param::MIRParametrisation& param) :
    Nearest(param) {
}


NearestNeighbour::~NearestNeighbour() {
}


const char *NearestNeighbour::name() const {
    return  "nearest-neighbour";
}


size_t NearestNeighbour::nclosest() const {
    return 1;
}


void NearestNeighbour::hash( eckit::MD5& md5) const {
    Nearest::hash(md5);
}


void NearestNeighbour::print(std::ostream& out) const {
    out << "NearestNeighbour[]";
}


namespace {
static MethodBuilder< NearestNeighbour > __knearest1("nearest-neighbour");
static MethodBuilder< NearestNeighbour > __knearest2("nearest-neighbor"); // For the americans
static MethodBuilder< NearestNeighbour > __knearest3("nn"); // For the lazy
}


}  // namespace method
}  // namespace mir


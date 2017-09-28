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


#include "mir/method/KNearest.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {


KNearest::KNearest(const param::MIRParametrisation& param) : KNearestNeighbours(param) {
}


KNearest::~KNearest() {
}


const char* KNearest::name() const {
    return "k-nearest";
}


namespace {
static MethodBuilder< KNearest > __method("k-nearest");
}


}  // namespace method
}  // namespace mir


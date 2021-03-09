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


#include "mir/key/packing/ArchivedValue.h"

#include "mir/key/packing/Packing.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace key {
namespace packing {


ArchivedValue::ArchivedValue(const param::MIRParametrisation& field) :
    defineAccuracy_(field.get("accuracy", accuracy_)),
    defineEdition_(field.get("edition", edition_)),
    definePacking_(field.get("packing", packing_)) {}


bool ArchivedValue::prepare(Packing& other) const {
    bool todo = false;

    long accuracy;
    if (other.getAccuracy(accuracy) && (!defineAccuracy_ || accuracy_ != accuracy)) {
        other.setAccuracy(accuracy);
        todo = true;
    }
    else {
        other.unsetAccuracy();
    }

    long edition;
    if (other.getEdition(edition) && (!defineEdition_ || edition_ != edition)) {
        other.setEdition(accuracy);
        todo = true;
    }
    else {
        other.unsetEdition();
    }

    std::string packing;
    if (other.getPacking(packing) && (!definePacking_ || packing_ != packing)) {
        other.setPacking(packing);
        todo = true;
    }
    else {
        other.unsetPacking();
    }

    return todo;
}


}  // namespace packing
}  // namespace key
}  // namespace mir

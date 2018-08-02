/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#include "mir/compare/FieldSet.h"
#include "eckit/log/Log.h"

#include <algorithm>


#include "mir/compare/BufrField.h"
#include "mir/compare/GribField.h"

namespace mir {
namespace compare {


void FieldSet::clear() {
    fields_.clear();
}

size_t FieldSet::size() const {
    return fields_.size();
}

void FieldSet::insert(const Field& field) {
    fields_.insert(field);
}


std::set<Field>::const_iterator FieldSet::duplicate(const Field& field) const {
    return fields_.find(field);
}


std::set<Field>::const_iterator FieldSet::same(const Field& field) const {
    auto j = fields_.find(field);
    if (j != fields_.end()) {
        return j;
    }

    for (j = fields_.begin(); j != fields_.end(); ++j) {
        if (field.same(*j)) {
            // eckit::Log::warning() << "Returning approximate match: " << field << std::endl;
            // eckit::Log::warning() << "                             " << *j << std::endl;
            return j;
        }
    }

    // eckit::Log::warning() << "Cannot find " << field << std::endl;
    // for (j = fields_.begin(); j != fields_.end(); ++j) {
    //     eckit::Log::warning() << *j << std::endl;
    //     eckit::Log::warning() << " " << ((*j) < field ) << " " << (field < (*j)) << std::endl;
    // }

    return end();
}


std::set<Field>::const_iterator FieldSet::end() const {
    return fields_.end();
}


std::set<Field>::const_iterator FieldSet::begin() const {
    return fields_.begin();
}


}  // namespace compare
}  // namespace mir


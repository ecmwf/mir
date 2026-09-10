// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/compare/FieldSet.h"


namespace mir::compare {


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
            return j;
        }
    }

    return end();
}


std::set<Field>::const_iterator FieldSet::end() const {
    return fields_.end();
}


std::set<Field>::const_iterator FieldSet::begin() const {
    return fields_.begin();
}


}  // namespace mir::compare

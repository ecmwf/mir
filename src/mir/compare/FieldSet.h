// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <set>

#include "mir/compare/Field.h"


namespace mir::compare {


class FieldSet {
public:  // methods
    void clear();
    size_t size() const;

    void insert(const Field& field);

    std::set<Field>::const_iterator same(const Field& field) const;

    std::set<Field>::const_iterator duplicate(const Field& field) const;


    std::set<Field>::const_iterator end() const;
    std::set<Field>::const_iterator begin() const;

private:  // members
    std::set<Field> fields_;
};


}  // namespace mir::compare

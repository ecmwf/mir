/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   FieldSet.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016


#ifndef mir_compare_FieldSet_h
#define mir_compare_FieldSet_h

//include <deque>

#include "mir/compare/Field.h"


namespace mir {
namespace compare {


class FieldSet {
public: // methods

    void clear();
    size_t size() const;

    void insert(const Field& field);

    std::set<Field>::const_iterator same(const Field& field) const;

    std::set<Field>::const_iterator duplicate(const Field& field) const;


    std::set<Field>::const_iterator end() const;
    std::set<Field>::const_iterator begin() const;

private: // members

    std::set<Field> fields_;

};


}  // namespace compare
}  // namespace mir


#endif


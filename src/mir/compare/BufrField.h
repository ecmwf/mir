/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   BufrField.h
/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   May 2016

#ifndef mir_compare_BufrField_H
#define mir_compare_BufrField_H

#include <iosfwd>
#include <string>
#include <set>
#include <vector>

#include "mir/compare/Field.h"
#include "mir/compare/FieldInfo.h"


namespace mir {
namespace compare {

class FieldSet;

//----------------------------------------------------------------------------------------------------------------------
//

class BufrField : public FieldBase {
public:

    BufrField(const std::string& path, off_t offset, size_t length);



    static void addOptions(std::vector<eckit::option::Option*>& options);
    static void setOptions(const eckit::option::CmdArgs &args);

private:

    bool operator==(const BufrField& other) const;


private:



    void print(std::ostream &out) const;

    friend std::ostream &operator<<(std::ostream &s, const BufrField &x) {
        x.print(s);
        return s;
    }

};


//----------------------------------------------------------------------------------------------------------------------

} // namespace mir
} // namespace compare

#endif

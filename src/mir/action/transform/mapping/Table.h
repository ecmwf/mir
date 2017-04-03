/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_action_transform_mapping_Table_h
#define mir_action_transform_mapping_Table_h

#include "mir/action/transform/mapping/Mapping.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace action {
namespace transform {
namespace mapping {


class Table : public Mapping {
public:

    // -- Exceptions
    // None

    // -- Contructors
    Table(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members
    // None

    // -- Methods
    void print(std::ostream&) const;

    // -- Overridden methods
    size_t getTruncationFromPointsPerLatitude(const size_t& N) const;
    size_t getPointsPerLatitudeFromTruncation(const size_t& T) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream& s, const Table& p) {
        p.print(s);
        return s;
    }

};


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir


#endif


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


#ifndef mir_key_packing_ArchivedValue_h
#define mir_key_packing_ArchivedValue_h

#include <string>


namespace mir {
namespace key {
namespace packing {
class Packing;
}
}  // namespace key
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir {
namespace key {
namespace packing {


class ArchivedValue {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    ArchivedValue(const param::MIRParametrisation& field);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    bool prepare(Packing&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
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

private:
    // -- Members

    long accuracy_;
    long edition_;
    std::string packing_;

    bool defineAccuracy_;
    bool defineEdition_;
    bool definePacking_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace packing
}  // namespace key
}  // namespace mir


#endif

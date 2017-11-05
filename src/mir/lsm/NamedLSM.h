/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date September 2017


#ifndef mir_lsm_NamedLSM_h
#define mir_lsm_NamedLSM_h

#include "mir/lsm/LSMSelection.h"


namespace mir {
namespace lsm {


class NamedLSM : public LSMSelection {
public:

    // -- Exceptions
    // None

    // -- Contructors

    NamedLSM(const std::string& name);

    // -- Destructor

    ~NamedLSM();

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

private:

    // -- Members
    // None

    // -- Methods

    std::string path(const param::MIRParametrisation&) const;

    // -- Overridden methods

    void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods

    Mask* create(const std::string& name,
                 const param::MIRParametrisation&,
                 const repres::Representation&,
                 const std::string& which) const;

    std::string cacheKey(const std::string& name,
                         const param::MIRParametrisation&,
                         const repres::Representation&,
                         const std::string& which) const;

    // -- Friends
    // None

};


}  // namespace lsm
}  // namespace mir


#endif


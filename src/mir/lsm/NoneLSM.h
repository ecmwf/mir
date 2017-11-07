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


#ifndef mir_lsm_NoneLSM_h
#define mir_lsm_NoneLSM_h

#include "mir/lsm/LSMSelection.h"


namespace mir {
namespace lsm {


class NoneLSM : public LSMSelection {
public:

    // -- Exceptions
    // None

    // -- Contructors

    NoneLSM(const std::string& name);

    // -- Destructor

    virtual ~NoneLSM();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static Mask& noMask();

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

    virtual void print(std::ostream&) const;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    virtual Mask* create(const std::string&,
                         const param::MIRParametrisation&,
                         const repres::Representation&,
                         const std::string& which) const ;

    virtual std::string cacheKey(const std::string&,
                                 const param::MIRParametrisation&,
                                 const repres::Representation&,
                                 const std::string& which) const;

    // -- Friends
    // None

};


}  // namespace lsm
}  // namespace mir


#endif


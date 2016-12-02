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


#ifndef AutoLSM_H
#define AutoLSM_H


#include "mir/lsm/LSMChooser.h"

namespace mir {
namespace lsm {


class AutoLSM : public LSMChooser {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    AutoLSM(const std::string &name);

    // -- Destructor

    virtual ~AutoLSM(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods


  protected:

    // -- Members

    // -- Methods


    virtual void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    AutoLSM(const AutoLSM &);
    AutoLSM &operator=(const AutoLSM &);

    // -- Members
    // None

    // -- Methods

    std::string path(const param::MIRParametrisation &param) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    virtual Mask *create(const std::string &,
                         const param::MIRParametrisation &param,
                         const atlas::grid::Grid &grid,
                         const std::string& which) const ;

    virtual std::string cacheKey(const std::string &,
                                 const param::MIRParametrisation &param,
                                 const atlas::grid::Grid &grid,
                                 const std::string& which) const ;


    // -- Friends



};


}  // namespace lsm
}  // namespace mir
#endif


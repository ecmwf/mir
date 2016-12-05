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


#ifndef mir_compare_Scalar_h
#define mir_compare_Scalar_h

#include "mir/compare/Comparator.h"


namespace mir {
namespace compare {


class Scalar : public Comparator {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Scalar(const param::MIRParametrisation &param1, const param::MIRParametrisation& param2);

    // -- Destructor

    virtual ~Scalar(); // Change to virtual if base class

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

    double absoluteError_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Scalar(const Scalar &);
    Scalar &operator=(const Scalar &);

    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods

    virtual void execute(const data::MIRField&, const data::MIRField&) const;

    virtual void print(std::ostream &) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace compare
}  // namespace mir


#endif

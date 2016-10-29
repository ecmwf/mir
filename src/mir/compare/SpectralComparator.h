/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Oct 2016


#ifndef mir_compare_SpectralComparator_H
#define mir_compare_SpectralComparator_H

#include "mir/compare/Comparator.h"


namespace mir {
namespace context {
class Context;
}
namespace data {
class MIRField;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class MIRStatistics;
}
}


namespace mir {
namespace compare {


class SpectralComparator : public Comparator {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    SpectralComparator(const param::MIRParametrisation &param1, const param::MIRParametrisation& param2);

    // -- Destructor

    virtual ~SpectralComparator(); // Change to virtual if base class

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

    SpectralComparator(const SpectralComparator &);
    SpectralComparator &operator=(const SpectralComparator &);

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

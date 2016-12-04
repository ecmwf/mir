/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Dec 2016


#ifndef mir_compare_Same_h
#define mir_compare_Same_h

#include "mir/compare/Comparator.h"


namespace mir {
namespace compare {
class Field;
}
}


namespace mir {
namespace compare {


class Same : public Comparator {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Same(const param::MIRParametrisation& param1, const param::MIRParametrisation& param2);

    // -- Destructor

    virtual ~Same(); // Change to virtual if base class

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

    bool sameAccuracy       (const Field&, const Field&) const;
    bool sameArea           (const Field&, const Field&) const;
    bool sameBitmap         (const Field&, const Field&) const;
    bool sameField          (const Field&, const Field&) const;
    bool sameFormat         (const Field&, const Field&) const;
    bool sameGridname       (const Field&, const Field&) const;
    bool sameGrid           (const Field&, const Field&) const;
    bool sameGridtype       (const Field&, const Field&) const;
    bool sameNumberOfPoints (const Field&, const Field&) const;
    bool samePacking        (const Field&, const Field&) const;
    bool sameParam          (const Field&, const Field&) const;
    bool sameResol          (const Field&, const Field&) const;
    bool sameRotation       (const Field&, const Field&) const;

    double compareAreas(const Field&, const Field&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Same(const Same&);
    Same& operator=(const Same&);

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

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


#ifndef MIRField_H
#define MIRField_H

#include <iosfwd>
#include <vector>


namespace mir {
namespace repres {
class Representation;
}
namespace data {


class MIRField {
  public:

// -- Exceptions
    // None

// -- Contructors

    //
    MIRField(bool hasMissing, double missingValue);

// -- Destructor

    ~MIRField(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    const std::vector<double>& values() const {
        return values_;
    }

    // Non-const version for direct update (Filter)
    std::vector<double>& values()  {
        return values_;
    }


    bool hasMissing() const {
        return hasMissing_;
    }

    double missingValue() const {
        return missingValue_;
    }


    // Warning Takes ownership of the vector
    void values(std::vector<double>&);

    void hasMissing(bool on) {
        hasMissing_ = on;
    }

    void missingValue(double value)  {
        missingValue_ = value;
    }

    void dimensions(size_t dimensions) {
        dimensions_ = dimensions;
    }

    size_t dimensions() const  {
        return dimensions_;
    }

    const repres::Representation* representation() const;
    void representation(repres::Representation*);

    //

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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed


// -- Members

    std::vector<double> values_;
    bool hasMissing_;
    double missingValue_;
    repres::Representation* representation_;
    size_t dimensions_; // 1 = Scalar, 2 = vector, erx

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const MIRField& p) {
        p.print(s);
        return s;
    }

};


}  // namespace data
}  // namespace mir
#endif


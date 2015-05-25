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
#include <cmath>


namespace mir {
namespace repres {
class Representation;
}
namespace param {
class MIRParametrisation;
}
namespace data {


class MIRField {
  public:

    struct Stats {

        double min;
        double max;
        double mean;
        double sqsum;
        double stdev;

        Stats() :
            min(std::numeric_limits<double>::max()),
            max(std::numeric_limits<double>::min()),
            mean(0.),
            sqsum(0.),
            stdev(0.)
        {}

        void print(std::ostream& s) const;

        friend std::ostream &operator<<(std::ostream &s, const Stats &p) {
            p.print(s);
            return s;
        }

    };

    // -- Exceptions
    // None

    // -- Contructors

    //
    MIRField(const param::MIRParametrisation&, bool hasMissing = false, double missingValue = 0);
    MIRField(repres::Representation*, bool hasMissing = false, double missingValue = 0);

    // -- Destructor

    ~MIRField(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t dimensions() const;


    void representation(repres::Representation *);
    const repres::Representation *representation() const;

    void values(std::vector<double> &, size_t which /*=0*/);  // Warning Takes ownership of the vector

    const std::vector<double> &values(size_t which /*=0*/) const;
    std::vector<double> &values(size_t which /*=0*/);   // Non-const version for direct update (Filter)

    void missingValue(double value);
    double missingValue() const;

    void hasMissing(bool on);
    bool hasMissing() const ;

    void validate() const;

    Stats statistics(size_t i) const;

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

    void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed


    // -- Members

    std::vector<std::vector<double> > values_;
    bool hasMissing_;
    double missingValue_;
    repres::Representation* representation_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const MIRField &p) {
        p.print(s);
        return s;
    }

};


}  // namespace data
}  // namespace mir
#endif


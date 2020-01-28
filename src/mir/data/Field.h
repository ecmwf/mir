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


#ifndef mir_data_Field_h
#define mir_data_Field_h

#include <iosfwd>
#include <vector>

#include "eckit/memory/Counted.h"
#include "mir/data/MIRValuesVector.h"


namespace mir {
namespace data {
class MIRFieldStats;
}
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
}


namespace mir {
namespace data {


class Field : public eckit::Counted {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Field(const param::MIRParametrisation&, bool hasMissing = false, double missingValue = 0);

    Field(const repres::Representation*, bool hasMissing = false, double missingValue = 0);

    // -- Destructor

    ~Field(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t dimensions() const;
    void dimensions(size_t);

    /// Resize to one, and keep only which
    void select(size_t which);

    void representation(const repres::Representation *);
    const repres::Representation *representation() const;

    /// @warning Takes ownership of the vector
    void update(MIRValuesVector&, size_t which, bool recomputeHasMissing = false);

    const MIRValuesVector& values(size_t which) const;
    MIRValuesVector& direct(size_t which);   // Non-const version for direct update (Filter)

    void metadata(size_t which, const std::map<std::string, long>&);
    void metadata(size_t which, const std::string& name, long value);
    const std::map<std::string, long>& metadata(size_t which) const;

    void missingValue(double value);
    double missingValue() const;

    void hasMissing(bool on);
    bool hasMissing() const;

    void validate() const;

    MIRFieldStats statistics(size_t i) const;

    /// @note not in MIRField
    Field* clone() const;

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

    Field(const Field& other);
    Field& operator=(const Field& other);

    // -- Members

    std::vector<MIRValuesVector> values_;
    std::vector<std::map<std::string, long> > metadata_;

    mutable bool recomputeHasMissing_;
    mutable bool hasMissing_;

    double missingValue_;
    const repres::Representation* representation_;

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Field &p) {
        p.print(s);
        return s;
    }

};


}  // namespace data
}  // namespace mir


#endif

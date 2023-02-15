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


#pragma once

#include <iosfwd>
#include <map>
#include <vector>

#include "eckit/memory/Counted.h"

#include "mir/util/Types.h"


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
}  // namespace mir


namespace mir::data {


class Field : public eckit::Counted {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Field(const param::MIRParametrisation&, bool hasMissing = false, double missingValue = 0);
    Field(const repres::Representation*, bool hasMissing = false, double missingValue = 0);

    // -- Destructor

    ~Field() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t dimensions() const;
    void dimensions(size_t);

    /// Resize to one, and keep only which
    void select(size_t which);

    void representation(const repres::Representation*);
    const repres::Representation* representation() const;

    /// @warning Takes ownership of the vector
    void update(MIRValuesVector&, size_t which, bool recomputeHasMissing = false);

    const MIRValuesVector& values(size_t which) const;
    MIRValuesVector& direct(size_t which);  // Non-const version for direct update (Filter)

    void metadata(size_t which, const std::map<std::string, long>&);
    void metadata(size_t which, const std::string& name, long value);
    const std::map<std::string, long>& metadata(size_t which) const;

    void missingValue(double value);
    double missingValue() const;

    void hasMissing(bool on);
    bool hasMissing() const;

    void validate() const;

    void handle(size_t which, size_t handle);
    size_t handle(size_t which) const;

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

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Constructors

    Field(const Field&);  // (use clone to copy)

    // -- Operators

    Field& operator=(const Field&);  // (use clone to copy)

    // -- Members

    std::vector<MIRValuesVector> values_;
    std::vector<std::map<std::string, long> > metadata_;
    std::map<size_t, size_t> handles_;

    double missingValue_;
    const repres::Representation* representation_;

    mutable bool recomputeHasMissing_;
    mutable bool hasMissing_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Field& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::data

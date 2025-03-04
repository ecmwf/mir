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
#include <string>


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {
class Representation;
}
namespace lsm {
class Mask;
}
}  // namespace mir


namespace mir::lsm {


class LSMSelection {
public:
    // -- Exceptions
    // None

    // -- Constructors

    LSMSelection(const LSMSelection&) = delete;
    LSMSelection(LSMSelection&&)      = delete;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const LSMSelection&) = delete;
    void operator=(LSMSelection&&)      = delete;

    // -- Methods

    virtual Mask* create(const param::MIRParametrisation&, const repres::Representation&,
                         const std::string& which) const = 0;

    virtual std::string cacheKey(const param::MIRParametrisation&, const repres::Representation&,
                                 const std::string& which) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const LSMSelection& lookup(const std::string& name);
    static void list(std::ostream&);

protected:
    // -- Constructors

    LSMSelection(const std::string& name);

    // -- Destructor

    virtual ~LSMSelection();

    // -- Members

    std::string name_;

    // -- Methods


    virtual void print(std::ostream&) const = 0;

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
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const LSMSelection& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::lsm

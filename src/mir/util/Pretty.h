/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_Pretty_h
#define mir_util_Pretty_h

#include <iosfwd>
#include <string>


namespace mir {
namespace util {


class Pretty {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    Pretty(int count) : count_(count) {}
    Pretty(size_t count) : Pretty(static_cast<int>(count)) {}

    Pretty(int count, std::string one) : s_{one, one + "s"}, count_(count) {}
    Pretty(size_t count, std::string one) : Pretty(static_cast<int>(count), one) {}

    Pretty(int count, std::string one, std::string notOne) : s_{one, notOne}, count_(count) {}
    Pretty(size_t count, std::string one, std::string notOne) : Pretty(static_cast<int>(count), one, notOne) {}

    Pretty(const Pretty&) = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    Pretty& operator=(const Pretty&) = delete;

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
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    const std::string s_[2];
    int count_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Pretty& p) {
        p.print(s);
        return s;
    }
};


}  // namespace util
}  // namespace mir


#endif

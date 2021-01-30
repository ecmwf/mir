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


#ifndef mir_util_Pretty_h
#define mir_util_Pretty_h

#include <iosfwd>
#include <string>

#include "mir/util/Log.h"


namespace mir {


class Pretty {
public:
    // -- Types

    using Plural = Log::Plural;

    // -- Exceptions
    // None

    // -- Constructors

    Pretty(int count, const Plural& plural = Plural()) : plural_(plural), count_(count) {}
    Pretty(long count, const Plural& plural = Plural()) : plural_(plural), count_(static_cast<int>(count)) {}
    Pretty(size_t count, const Plural& plural = Plural()) : plural_(plural), count_(static_cast<int>(count)) {}
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

    const Plural& plural_;
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


}  // namespace mir


#endif

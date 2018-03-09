/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_predicate_Predicate_h
#define mir_util_predicate_Predicate_h

#include <iosfwd>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace util {
namespace predicate {


class Predicate {
public:

    Predicate();
    virtual ~Predicate();

private:

    virtual void print(std::ostream& s) const = 0;

    friend std::ostream& operator<<(std::ostream& out, const Predicate& f) {
        f.print(out);
        return out;
    }

};


}  // namespace predicate
}  // namespace util
}  // namespace mir


#endif

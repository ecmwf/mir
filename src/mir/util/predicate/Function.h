/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_predicate_Function_h
#define mir_util_predicate_Function_h

#include <string>
#include <vector>

#include "mir/util/predicate/Predicate.h"
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace util {
namespace predicate {


class Function : public eckit::NonCopyable {
public:

    Function(const std::string& name);
    virtual ~Function();

    virtual void execute(context::Context& ctx) const = 0;


    static const Function& lookup(const std::string& name);

private:

    virtual void print(std::ostream& s) const = 0;

protected:

    const std::string name_;

private:


    friend std::ostream &operator<<(std::ostream &s, const Function &p) {
        p.print(s);
        return s;
    }

};

}
}
} // namespace eckit

#endif

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_predicate_PredicateFunction_h
#define mir_util_predicate_PredicateFunction_h

#include <string>
#include <vector>

#include "mir/util/predicate/Predicate.h"


namespace mir {
namespace util {
namespace predicate {
class Function;
}
}
}


namespace mir {
namespace util {
namespace predicate {


class PredicateFunction : public Predicate {
public:

    PredicateFunction(const std::string& name, Predicate*);
    PredicateFunction(const std::string& name, Predicate*, Predicate*);
    PredicateFunction(const std::string& name, std::vector<Predicate*>&);

    virtual ~PredicateFunction();

protected:

    const Function& function_;
    std::vector<Predicate*> args_;

private:

    virtual void print(std::ostream& s) const;
    virtual void execute(context::Context & ctx) const;
    virtual const char* name() const;

};



}  // namespace predicate
}  // namespace util
}  // namespace mir


#endif

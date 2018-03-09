/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_predicate_PredicateNumber_h
#define mir_util_predicate_PredicateNumber_h

#include "mir/util/predicate/Predicate.h"


namespace mir {
namespace util {
namespace predicate {


class PredicateNumber : public Predicate {
public:

    PredicateNumber(double value);
    virtual ~PredicateNumber();

private:

    virtual void print(std::ostream& s) const;
    virtual void execute(context::Context & ctx) const;
    virtual const char* name() const;

private: // members

    double value_;

};


}  // namespace predicate
}  // namespace util
}  // namespace mir


#endif

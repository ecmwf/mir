/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef VectorOfUnknownLengthOption_H
#define VectorOfUnknownLengthOption_H

#include <iosfwd>
#include "eckit/memory/NonCopyable.h"
#include "eckit/option/SimpleOption.h"


namespace mir {
namespace util {
namespace option {


using eckit::Configuration;
using eckit::Configured;
using eckit::NonCopyable;
using namespace eckit::option;


template<class T>
class VectorOfUnknownLengthOption : public Option, private NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    VectorOfUnknownLengthOption(const std::string& name, const std::string& description, const char* separator="/");

    // -- Destructor

    virtual ~VectorOfUnknownLengthOption();

    // -- Convertors
    // None

    // -- Operators
    // None

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

    virtual void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    mutable size_t size_;
    const char* separator_;

    // -- Methods
    // None

    // -- Overridden methods

    using Option::set;
    virtual void set(Configured&) const;
    virtual void set(const std::string& value, Configured&) const;
    virtual void copy(const Configuration& from, Configured& to) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace option
}  // namespace util
}  // namespace mir


#include "mir/util/option/VectorOfUnknownLengthOption.cc"
#endif


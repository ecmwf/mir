/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#ifndef mir_action_transform_CompressIf_h
#define mir_action_transform_CompressIf_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace util {
class BoundingBox;
}
}


namespace mir {
namespace action {
namespace transform {


class CompressIf : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    CompressIf(const param::MIRParametrisation&) {}

    // -- Destructor

    virtual ~CompressIf() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual bool operator()(const util::BoundingBox& cropping) const = 0;
    virtual void print(std::ostream&) const = 0;

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

    friend std::ostream& operator<<(std::ostream& s, const CompressIf& p) {
        p.print(s);
        return s;
    }

};


class CompressIfFactory {
    std::string name_;
    virtual CompressIf* make(const param::MIRParametrisation&) = 0;
protected:
    CompressIfFactory(const std::string&);
    virtual ~CompressIfFactory();
public:
    static CompressIf* build(const std::string&, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template<class T>
class CompressIfBuilder : public CompressIfFactory {
    CompressIf* make(const param::MIRParametrisation& parametrisation) {
        return new T(parametrisation);
    }
public:
    CompressIfBuilder(const std::string& name) : CompressIfFactory(name) {}
};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif

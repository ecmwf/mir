/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#ifndef mir_style_Resol_h
#define mir_style_Resol_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace action {
class ActionPlan;
}
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace style {


class Resol : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Resol(const param::MIRParametrisation& parametrisation) :
        parametrisation_(parametrisation) {
    }

    // -- Destructor

    virtual ~Resol() {}

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual void prepare(action::ActionPlan&) const = 0;
    virtual bool resultIsSpectral() const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:

    // -- Members

    const param::MIRParametrisation& parametrisation_;

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

    friend std::ostream& operator<<(std::ostream& s, const Resol& p) {
        p.print(s);
        return s;
    }
};


class ResolFactory {
    std::string name_;
    virtual Resol *make(const param::MIRParametrisation&) = 0;
protected:
    ResolFactory(const std::string&);
    virtual ~ResolFactory();
public:
    static Resol *build(const std::string&, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T> class ResolBuilder : public ResolFactory {
    virtual Resol *make(const param::MIRParametrisation& parametrisation) {
        return new T(parametrisation);
    }
public:
    ResolBuilder(const std::string& name) : ResolFactory(name) {}
};


}  // namespace style
}  // namespace mir


#endif

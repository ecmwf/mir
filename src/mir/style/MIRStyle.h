/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_style_MIRStyle_h
#define mir_style_MIRStyle_h

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace action {
class ActionPlan;
}
namespace api {
class MIRJob;
}
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace style {


class MIRStyle : private eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    MIRStyle(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~MIRStyle(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual void prepare(action::ActionPlan&) const = 0;

    virtual bool forcedPrepare(const api::MIRJob&, const param::MIRParametrisation& input) const = 0;

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

    friend std::ostream& operator<<(std::ostream& s,const MIRStyle& p) {
        p.print(s);
        return s;
    }

};


class MIRStyleFactory {
    std::string name_;
    virtual MIRStyle* make(const param::MIRParametrisation&) = 0 ;

protected:

    MIRStyleFactory(const std::string&);
    virtual ~MIRStyleFactory();

public:
    static MIRStyle* build(const param::MIRParametrisation&);
    static void list(std::ostream &);

};


template<class T>
class MIRStyleBuilder : public MIRStyleFactory {
    virtual MIRStyle* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
public:
    MIRStyleBuilder(const std::string& name) : MIRStyleFactory(name) {}
};


}  // namespace style
}  // namespace mir


#endif


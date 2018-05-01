/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
/// @date   Apr 2015


#ifndef mir_action_plan_Action_h
#define mir_action_plan_Action_h

#include <iosfwd>
#include <string>

#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class BoundingBox;
class MIRStatistics;
}
}


namespace mir {
namespace action {


class Action : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Action(const param::MIRParametrisation &parametrisation);

    // -- Destructor

    virtual ~Action(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void perform(context::Context & ctx) const;


    virtual bool sameAs(const Action& other) const = 0;
    virtual void custom(std::ostream &) const; // Change to virtual if base class
    virtual const char* name() const = 0;

    // For optimising plans
    virtual bool mergeWithNext(const Action& other);
    virtual bool isCropAction() const;
    virtual bool canCrop() const;
    virtual const util::BoundingBox& croppingBoundingBox() const;
    virtual util::BoundingBox extendedBoundingBox(const util::BoundingBox&, double angle) const;

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

    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

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

    // call perform() for the public interface
    virtual void execute(context::Context&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Action &p) {
        p.print(s);
        return s;
    }

};


class ActionFactory {

    std::string name_;

    virtual Action *make(const param::MIRParametrisation &) = 0;

protected:

    ActionFactory(const std::string &);

    virtual ~ActionFactory();

public:

    static Action *build(const std::string&, const param::MIRParametrisation&, bool exact = true);

    static void list(std::ostream&);

};


template<class T>
class ActionBuilder : public ActionFactory {
    virtual Action *make(const param::MIRParametrisation &param) {
        return new T(param);
    }
public:
    ActionBuilder(const std::string &name) : ActionFactory(name) {}
};


}  // namespace action
}  // namespace mir


#endif

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


#ifndef Action_H
#define Action_H

#include <string>


namespace mir {

namespace data {
class MIRField;
}

namespace param {
class MIRParametrisation;
}

namespace action {


class Action {
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

    virtual void execute(data::MIRField &) const = 0;
    virtual bool sameAs(const Action& other) const = 0;
    virtual bool needField() const ;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members

    const param::MIRParametrisation &parametrisation_;

    // -- Methods

    virtual void print(std::ostream &) const = 0; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    Action(const Action &);
    Action &operator=(const Action &);

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

    static Action *build(const std::string &, const param::MIRParametrisation &);

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


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


#ifndef MIRLogic_H
#define MIRLogic_H

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>


namespace mir {
namespace action {
class Action;
}
namespace param {
class MIRParametrisation;
}
namespace logic {


class MIRLogic {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIRLogic(const param::MIRParametrisation& parametrisation);

// -- Destructor

    virtual ~MIRLogic(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void prepare(std::vector<std::auto_ptr< action::Action > >&) const = 0;

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

    void add(std::vector<std::auto_ptr< action::Action > >& actions, const std::string& name) const;


    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    MIRLogic(const MIRLogic&);
    MIRLogic& operator=(const MIRLogic&);

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

    friend std::ostream& operator<<(std::ostream& s,const MIRLogic& p) {
        p.print(s);
        return s;
    }

};


class MIRLogicFactory {
    std::string name_;
    virtual MIRLogic* make(const param::MIRParametrisation&) = 0 ;

  protected:

    MIRLogicFactory(const std::string&);
    virtual ~MIRLogicFactory();

  public:
    static MIRLogic* build(const param::MIRParametrisation&);

};


template<class T>
class MIRLogicBuilder : public MIRLogicFactory {
    virtual MIRLogic* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
  public:
    MIRLogicBuilder(const std::string& name) : MIRLogicFactory(name) {}
};


}  // namespace logic
}  // namespace mir
#endif


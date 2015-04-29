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


#ifndef soyuz_method_Method_H
#define soyuz_method_Method_H

#include <iosfwd>
#include <string>
#include <vector>


namespace atlas {
class Grid;
}

namespace mir {
namespace data {
class MIRField;
}
namespace param {
class MIRParametrisation;
}
namespace method {


class Method {
  public:

// -- Exceptions
    // None

// -- Contructors
    Method(const param::MIRParametrisation&);

// -- Destructor
    virtual ~Method();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    virtual void execute(data::MIRField&, const atlas::Grid&, const atlas::Grid&) const = 0;

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

// No copy allowed

    Method(const Method&);
    Method& operator=(const Method&);

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

    friend std::ostream& operator<<(std::ostream& s,const Method& p) {
        p.print(s);
        return s;
    }

};


class MethodFactory {
    std::string name_;
    virtual Method* make(const param::MIRParametrisation&) = 0;

  protected:

    MethodFactory(const std::string&);
    virtual ~MethodFactory();

  public:

    static Method* build(const std::string&, const param::MIRParametrisation&);

};


template< class T>
class MethodBuilder : public MethodFactory {
    virtual Method* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
  public:
    MethodBuilder(const std::string& name) : MethodFactory(name) {}
};


}  // namespace method
}  // namespace mir
#endif


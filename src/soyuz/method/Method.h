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

#include <string>
#include <iosfwd>


class MIRParametrisation;
class MIRField;


namespace mir {
namespace method {


class Method {
  public:

// -- Exceptions
    // None

// -- Contructors

    Method();

// -- Destructor

    virtual ~Method(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void execute(MIRField&) const = 0;

    const std::string& name() const {
        return name_;
    }

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members


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

    // name created methods, allowing builder to name them
    std::string name_;
    std::string& name() {
        return name_;
    }
    template< class T > friend class MethodBuilder;

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
    virtual Method* make(const std::string&, const MIRParametrisation&) = 0 ;

  protected:

    MethodFactory(const std::string&);
    virtual ~MethodFactory();

  public:
    static Method* build(const std::string&, const MIRParametrisation&);

};


template<class T>
class MethodBuilder : public MethodFactory {
    virtual Method* make(const std::string& name, const MIRParametrisation& param) {
        T* baby = new T(param);
        baby->name() = name;
        return baby;
    }
  public:
    MethodBuilder(const std::string& name) : MethodFactory(name) {}
};


}  // namespace method
}  // namespace mir
#endif


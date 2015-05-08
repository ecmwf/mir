/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_redist_Redist_H
#define mir_method_redist_Redist_H

#include <string>
#if 0
#include <iosfwd>
#endif


//namespace atlas {
//class Grid;
//}

namespace mir {
//namespace data {
//class MIRField;
//}
namespace param {
class MIRParametrisation;
}
namespace method {
namespace redist {


class Redist {
  public:

// -- Exceptions
    // None

// -- Contructors
    Redist(const param::MIRParametrisation&);

// -- Destructor
    virtual ~Redist();

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods
    virtual void execute(/*data::MIRField&, const atlas::Grid&, const atlas::Grid&*/) const = 0;

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

    Redist(const Redist&);
    Redist& operator=(const Redist&);

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

    friend std::ostream& operator<<(std::ostream& s, const Redist& p) {
        p.print(s);
        return s;
    }

};


class RedistFactory {
    std::string name_;
    virtual Redist* make(const param::MIRParametrisation&) = 0;

  protected:

    RedistFactory(const std::string&);
    virtual ~RedistFactory();

  public:

    static Redist* build(const std::string&, const param::MIRParametrisation&);

};


template< class T>
class RedistBuilder : public RedistFactory {
    virtual Redist* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
  public:
    RedistBuilder(const std::string& name) : RedistFactory(name) {}
};


}  // namespace redist
}  // namespace method
}  // namespace mir
#endif


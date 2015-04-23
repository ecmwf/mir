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


#ifndef Representation_H
#define Representation_H

#include <iosfwd>
#include <string>
#include <vector>


namespace atlas {
class Grid;
}

struct grib_info;


namespace mir {
namespace param {
class MIRParametrisation;
}
namespace repres {


class Representation {
  public:

// -- Exceptions
    // None

// -- Contructors

    Representation();

// -- Destructor

    virtual ~Representation(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void fill(grib_info&) const;

    virtual Representation* crop(double north, double west, double south, double east,
                                 const std::vector<double>&, std::vector<double>&) const;

    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

    virtual Representation* truncate(size_t truncation,
                                     const std::vector<double>&, std::vector<double>&) const;

    virtual atlas::Grid* atlasGrid() const;

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

    Representation(const Representation&);
    Representation& operator=(const Representation&);

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

    friend std::ostream& operator<<(std::ostream& s,const Representation& p) {
        p.print(s);
        return s;
    }

};


class RepresentationFactory {
    std::string name_;
    virtual Representation* make(const param::MIRParametrisation&) = 0 ;

  protected:

    RepresentationFactory(const std::string&);
    virtual ~RepresentationFactory();

  public:
    static Representation* build(const param::MIRParametrisation&);

};


template<class T>
class RepresentationBuilder : public RepresentationFactory {
    virtual Representation* make(const param::MIRParametrisation& param) {
        return new T(param);
    }
  public:
    RepresentationBuilder(const std::string& name) : RepresentationFactory(name) {}
};


}  // namespace repres
}  // namespace mir
#endif


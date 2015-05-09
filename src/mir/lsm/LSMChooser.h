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


#ifndef LSMChooser_H
#define LSMChooser_H

#include <string>
#include <iosfwd>

namespace atlas { class Grid; }
namespace mir { namespace param { class MIRParametrisation; }}

namespace mir {
namespace lsm {

class Mask;

class LSMChooser {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    LSMChooser(const std::string &name);

    // -- Destructor

    virtual ~LSMChooser(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual Mask *create(const std::string &name, const std::string &key,
                         const param::MIRParametrisation &parametrisation,
                         const atlas::Grid &grid) const = 0 ;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static LSMChooser &lookup(const std::string& name);

  protected:

    // -- Members

    std::string name_;

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

    LSMChooser(const LSMChooser &);
    LSMChooser &operator=(const LSMChooser &);

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const LSMChooser &p) {
        p.print(s);
        return s;
    }

};



}  // namespace logic
}  // namespace mir
#endif


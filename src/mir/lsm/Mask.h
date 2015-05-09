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


#ifndef Mask_H
#define Mask_H

#include <iosfwd>
#include <string>
#include <memory>

namespace atlas { class Grid; }

namespace mir { namespace param { class MIRParametrisation; } }
namespace mir { namespace data { class MIRField; }}


namespace mir {
namespace lsm {

class Mask {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Mask(const std::string &name, const std::string &key);

    // -- Destructor

    virtual ~Mask(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual bool active() const;
    virtual std::string uniqueID() const;

    virtual bool cacheable() const;
    virtual const data::MIRField &field() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static  Mask &lookupInput(const param::MIRParametrisation &param, const atlas::Grid &grid);
    static  Mask &lookupOutput(const param::MIRParametrisation &param, const atlas::Grid &grid);

  protected:

    // -- Members

    std::string name_;
    std::string key_;
    std::auto_ptr<data::MIRField> field_;

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

    Mask(const Mask &);
    Mask &operator=(const Mask &);

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    static  Mask &lookup(const param::MIRParametrisation &param, const atlas::Grid &grid, const std::string& which);


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Mask &p) {
        p.print(s);
        return s;
    }

};


}  // namespace logic
}  // namespace mir
#endif


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


#ifndef LandSeaMask_H
#define LandSeaMask_H

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

// #include "mir/data/MIRField.h"

namespace atlas {
class Grid;
}


namespace mir {

namespace data {
class MIRField;
}
namespace action {
class Action;
class ActionPlan;
}

namespace param {
class MIRParametrisation;
class RuntimeParametrisation;
}

namespace lsm {


class LandSeaMask {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    LandSeaMask(const std::string &name, const std::string &key);

    // -- Destructor

    virtual ~LandSeaMask(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual bool active() const;
    virtual std::string unique_id() const;

    virtual bool cacheable() const;
    virtual const data::MIRField &field() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static  LandSeaMask &lookupInput(const param::MIRParametrisation &param, const atlas::Grid &grid);
    static  LandSeaMask &lookupOutput(const param::MIRParametrisation &param, const atlas::Grid &grid);

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

    LandSeaMask(const LandSeaMask &);
    LandSeaMask &operator=(const LandSeaMask &);

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    static  LandSeaMask &lookup(const param::MIRParametrisation &param, const atlas::Grid &grid, const std::string& which);


    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const LandSeaMask &p) {
        p.print(s);
        return s;
    }

};


}  // namespace logic
}  // namespace mir
#endif


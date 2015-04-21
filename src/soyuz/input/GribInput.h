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


#ifndef GribInput_H
#define GribInput_H

#include <map>
#include <memory>
#include <string>

#include "soyuz/input/MIRInput.h"
#include "soyuz/param/FieldParametrisation.h"


namespace mir {
namespace input {


class GribInput : public MIRInput, public FieldParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribInput();

    // -- Destructor

    virtual ~GribInput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    bool next();


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members



    // -- Methods

    bool handle(grib_handle*);


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    GribInput(const GribInput &);
    GribInput &operator=(const GribInput &);

    // -- Members

    std::auto_ptr<grib_handle> grib_;
    mutable std::map<std::string, std::string> cache_;

    // -- Methods


    // -- Overridden methods
    // From MIRInput

    virtual const MIRParametrisation &parametrisation() const;
    virtual MIRField *field() const;
    virtual grib_handle *gribHandle() const;

    // From FieldParametrisation
    bool lowLevelGet(const std::string &name, std::string &value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif


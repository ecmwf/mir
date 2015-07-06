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


#ifndef RawInput_H
#define RawInput_H


#include "mir/input/MIRInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/api/ProdgenJob.h"


namespace mir {
namespace input {


class RawInput : public MIRInput, public param::MIRParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors
    RawInput(const api::ProdgenJob& metadata, const double* values, size_t count);

    // -- Destructor

    virtual ~RawInput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    RawInput(const RawInput &);
    RawInput &operator=(const RawInput &);

    // -- Members

    const api::ProdgenJob& metadata_;
    const double* values_;
    size_t count_;

    // -- Methods

    // -- Overridden methods
    // From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class

    virtual const param::MIRParametrisation &parametrisation() const;
    virtual data::MIRField *field() const;

    virtual bool next();

    // From MIRParametrisation
    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, double& value) const;

    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const RawInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif


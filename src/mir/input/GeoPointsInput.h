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


#ifndef GeoPointsInput_H
#define GeoPointsInput_H


#include "mir/input/MIRInput.h"
#include "mir/param/FieldParametrisation.h"


namespace mir {
namespace input {


class GeoPointsInput : public MIRInput, public param::FieldParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors
    GeoPointsInput(const std::string& path);

    // -- Destructor

    virtual ~GeoPointsInput(); // Change to virtual if base class

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

    GeoPointsInput(const GeoPointsInput &);
    GeoPointsInput &operator=(const GeoPointsInput &);

    // -- Members

    std::string path_;

    mutable std::vector<double> latitudes_;
    mutable std::vector<double> longitudes_;
    mutable std::vector<double> values_;

    // -- Methods

    // -- Overridden methods
    // From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const;

    virtual const param::MIRParametrisation &parametrisation() const;
    virtual data::MIRField *field() const;

    virtual bool next();

    // From MIRParametrisation
    virtual bool has(const std::string& name) const;
    virtual bool get(const std::string&, std::string&) const;

    // From FieldParametrisation
    virtual void latitudes(std::vector<double> &) const;
    virtual void longitudes(std::vector<double> &) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GeoPointsInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif


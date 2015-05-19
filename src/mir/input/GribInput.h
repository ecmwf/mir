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


#include "mir/input/MIRInput.h"
#include "mir/param/FieldParametrisation.h"


namespace mir {
namespace input {


class GribInput : public MIRInput, public param::FieldParametrisation {
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

    virtual bool next();

    void setAuxilaryFiles(const std::string &pathToLatitudes, const std::string &pathToLongitudes);


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members



    // -- Methods

    bool handle(grib_handle *);


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

    grib_handle *grib_;

    // --- For unstructured grids


    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods

    void auxilaryValues(const std::string&, std::vector<double> &values) const;


    // -- Overridden methods
    // From MIRInput

    virtual const param::MIRParametrisation &parametrisation() const;
    virtual data::MIRField *field() const;
    virtual grib_handle *gribHandle() const;

    /// From MIRParametrisation
    virtual bool get(const std::string &name, std::string &value) const;
    virtual bool get(const std::string &name, bool &value) const;
    virtual bool get(const std::string &name, long &value) const;
    virtual bool get(const std::string &name, double &value) const;
    virtual bool get(const std::string &name, std::vector<long> &value) const;
    virtual bool get(const std::string &name, std::vector<double> &value) const;

    virtual bool has(const std::string &name) const;

    // From FieldParametrisation
    virtual void latitudes(std::vector<double> &) const;
    virtual void longitudes(std::vector<double> &) const;


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


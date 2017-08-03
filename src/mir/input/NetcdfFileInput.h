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


#ifndef NetcdfFileInput_H
#define NetcdfFileInput_H

#include "mir/api/mir_config.h"
#ifdef HAVE_NETCDF

#include <string>

#include "eckit/filesystem/PathName.h"

#include "mir/input/MIRInput.h"
#include "mir/param/FieldParametrisation.h"
#include "mir/netcdf/NCFileCache.h"
#include "mir/netcdf/InputDataset.h"
#include "mir/param/CachedParametrisation.h"


namespace mir {
namespace input {


class NetcdfFileInput : public MIRInput,
    public param::FieldParametrisation,
    public mir::netcdf::NCFileCache {
public:

    // -- Exceptions
    // None

    // -- Contructors

    NetcdfFileInput(const eckit::PathName&);

    // -- Destructor

    virtual ~NetcdfFileInput(); // Change to virtual if base class

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

    NetcdfFileInput(const NetcdfFileInput &);
    NetcdfFileInput &operator=(const NetcdfFileInput &);

    // -- Members

    eckit::PathName path_;

    param::CachedParametrisation cache_;
    mir::netcdf::InputDataset dataset_;
    std::vector<mir::netcdf::Field*> fields_;
    int current_;

    // mutable std::vector<double> latitude_;
    // mutable std::vector<double> longitude_;

    // -- Methods


    // -- Overridden methods
    // From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const;
    virtual const param::MIRParametrisation &parametrisation(size_t which) const;
    virtual data::MIRField field() const;

    // From MIRParametrisation
    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string&, long&) const;
    virtual bool get(const std::string&, std::string&) const;
    virtual bool get(const std::string &name, double &value) const;
    virtual bool get(const std::string &name, std::vector<double> &value) const;

    virtual bool next();
    virtual size_t dimensions() const;

    virtual grib_handle *gribHandle(size_t which) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const NetcdfFileInput& p)
    //  { p.print(s); return s; }

};


}  // namespace input
}  // namespace mir
#endif
#endif


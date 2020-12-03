/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_input_NetcdfFileInput_h
#define mir_input_NetcdfFileInput_h

#include "mir/api/mir_config.h"
#if defined(mir_HAVE_NETCDF)

#include <string>

#include "eckit/filesystem/PathName.h"

#include "mir/input/MIRInput.h"
#include "mir/netcdf/InputDataset.h"
#include "mir/netcdf/NCFileCache.h"
#include "mir/param/CachedParametrisation.h"
#include "mir/param/FieldParametrisation.h"


namespace mir {
namespace input {


class NetcdfFileInput : public MIRInput, public param::FieldParametrisation, public netcdf::NCFileCache {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NetcdfFileInput(const eckit::PathName&);

    // -- Destructor

    virtual ~NetcdfFileInput() override;  // Change to virtual if base class

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    eckit::PathName path_;
    param::CachedParametrisation cache_;

    netcdf::InputDataset dataset_;
    std::vector<netcdf::Field*> fields_;
    int current_;

    // mutable std::vector<double> latitude_;
    // mutable std::vector<double> longitude_;

    // -- Methods

    const netcdf::Field& currentField() const;

    // -- Overridden methods

    // From MIRInput
    virtual void print(std::ostream&) const override;  // Change to virtual if base class
    virtual bool sameAs(const MIRInput& other) const override;
    virtual const param::MIRParametrisation& parametrisation(size_t which) const;
    virtual data::MIRField field() const;
    virtual bool next();
    virtual size_t dimensions() const;
    virtual grib_handle* gribHandle(size_t which) const;

    // From FieldParametrisation
    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, int& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, float& value) const;
    virtual bool get(const std::string& name, double& value) const;

    virtual bool get(const std::string& name, std::vector<int>& value) const;
    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<float>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;
    virtual bool get(const std::string& name, std::vector<std::string>& value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    // friend ostream& operator<<(ostream& s,const NetcdfFileInput& p)
    //  { p.print(s); return s; }
};


}  // namespace input
}  // namespace mir


#endif
#endif

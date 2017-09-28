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


#ifndef mir_input_GribInput_h
#define mir_input_GribInput_h

#include "eckit/thread/Mutex.h"
#include "mir/input/MIRInput.h"
#include "mir/param/CachedParametrisation.h"
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

    virtual ~GribInput();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void setAuxilaryFiles(const std::string& pathToLatitudes, const std::string& pathToLongitudes);

    // For debugging only
    void marsRequest(std::ostream&) const;

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

    bool handle(grib_handle*);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    param::CachedParametrisation cache_;

    mutable eckit::Mutex mutex_;
    grib_handle *grib_;

    // For unstructured grids
    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Methods

    void auxilaryValues(const std::string&, std::vector<double>& values) const;

    // -- Overridden methods

    // From MIRInput
    virtual const param::MIRParametrisation& parametrisation(size_t which) const;
    virtual data::MIRField field() const;
    virtual grib_handle *gribHandle(size_t which = 0) const;

    /// From MIRParametrisation
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

    // From FieldParametrisation
    virtual void latitudes(std::vector<double>&) const;
    virtual void longitudes(std::vector<double>&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace input
}  // namespace mir


#endif


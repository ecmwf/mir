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


#pragma once

#include "mir/input/MIRInput.h"
#include "mir/param/CachedParametrisation.h"
#include "mir/param/FieldParametrisation.h"
#include "mir/util/Mutex.h"


namespace mir::input {


class GribInput : public MIRInput, public param::FieldParametrisation {
public:
    GribInput();

    ~GribInput() override;

    // For debugging only
    void marsRequest(std::ostream&) const;

protected:
    bool handle(grib_handle*);

private:
    // -- Members

    param::CachedParametrisation cache_;

    mutable util::recursive_mutex mutex_;

    grib_handle* grib_;

    // For unstructured grids
    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    // -- Overridden methods

    // From MIRInput
    const param::MIRParametrisation& parametrisation(size_t which) const override;
    data::MIRField field() const override;
    grib_handle* gribHandle(size_t which = 0) const override;
    void setAuxiliaryInformation(const util::ValueMap&) override;
    bool only(size_t paramId) override;
    size_t dimensions() const override;

    /// From MIRParametrisation
    bool has(const std::string& name) const override;

    bool get(const std::string& name, std::string& value) const override;
    bool get(const std::string& name, bool& value) const override;
    bool get(const std::string& name, int& value) const override;
    bool get(const std::string& name, long& value) const override;
    bool get(const std::string& name, float& value) const override;
    bool get(const std::string& name, double& value) const override;

    bool get(const std::string& name, std::vector<int>& value) const override;
    bool get(const std::string& name, std::vector<long>& value) const override;
    bool get(const std::string& name, std::vector<float>& value) const override;
    bool get(const std::string& name, std::vector<double>& value) const override;
    bool get(const std::string& name, std::vector<std::string>& value) const override;

    // From FieldParametrisation
    void latitudes(std::vector<double>&) const override;
    void longitudes(std::vector<double>&) const override;
};


}  // namespace mir::input

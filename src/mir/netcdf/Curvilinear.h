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

#include "mir/netcdf/GridSpec.h"


namespace mir::netcdf {


class Curvilinear : public GridSpec {
public:
    Curvilinear(const Variable&, const std::vector<double>& latitudes, const std::vector<double>& longitudes);

    ~Curvilinear() override;

    // -- Methods

    static GridSpec* guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes);


protected:
    // -- Members

    std::vector<double> latitudes_;
    std::vector<double> longitudes_;

    double north_;
    double west_;
    double south_;
    double east_;

private:
    Curvilinear(const Curvilinear&);
    Curvilinear& operator=(const Curvilinear&);

    // - Methods

    void print(std::ostream&) const override;

    // From GridSpec
    bool has(const std::string& name) const override;
    bool get(const std::string& name, long&) const override;
    bool get(const std::string& name, std::string&) const override;
    bool get(const std::string& name, double&) const override;
    bool get(const std::string& name, std::vector<double>&) const override;
    void reorder(MIRValuesVector&) const override;
};


}  // namespace mir::netcdf

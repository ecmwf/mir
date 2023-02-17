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


class Rectilinear : public GridSpec {
public:
    Rectilinear(const Variable&, double north, double south, const std::vector<double>& latitudes, double west,
                double east, const std::vector<double>& longitudes);

    ~Rectilinear() override;

    // -- Methods

    static GridSpec* guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes);


protected:
    // -- Members

    double north_;
    double south_;
    std::vector<double> latitudes_;

    double west_;
    double east_;
    std::vector<double> longitudes_;

    bool jScansPositively_;

private:
    Rectilinear(const Rectilinear&);
    Rectilinear& operator=(const Rectilinear&);

    // - Methods

    void print(std::ostream&) const override;

    // From GridSpec
    bool has(const std::string& name) const override;
    bool get(const std::string&, long&) const override;
    bool get(const std::string&, std::string&) const override;
    bool get(const std::string& name, double& value) const override;
    bool get(const std::string& name, std::vector<double>& value) const override;
    void reorder(MIRValuesVector& values) const override;
};


}  // namespace mir::netcdf

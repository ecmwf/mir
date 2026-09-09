// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/GridSpec.h"


namespace mir::netcdf {


class UnstructuredGrid : public GridSpec {
public:
    UnstructuredGrid(const Variable&, const std::vector<double>& latitudes, const std::vector<double>& longitudes);

    ~UnstructuredGrid() override;

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
    UnstructuredGrid(const UnstructuredGrid&);
    UnstructuredGrid& operator=(const UnstructuredGrid&);

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

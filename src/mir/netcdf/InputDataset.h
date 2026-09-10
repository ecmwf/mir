// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Dataset.h"


namespace mir::netcdf {
class Field;
class NCFileCache;
}  // namespace mir::netcdf


namespace mir::netcdf {


class InputDataset : public Dataset {
public:
    InputDataset(const std::string&, NCFileCache&);
    ~InputDataset() override;

    virtual std::vector<Field*> fields() const;

private:
    InputDataset(const InputDataset&);
    InputDataset& operator=(const InputDataset&);

    // -- Members

    int number_of_dimensions_;
    int number_of_variables_;
    int number_of_global_attributes_;
    int id_of_unlimited_dimension_;

    int format_;
    NCFileCache& cache_;

    // - Methods

    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf

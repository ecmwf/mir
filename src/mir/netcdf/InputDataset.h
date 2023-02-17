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

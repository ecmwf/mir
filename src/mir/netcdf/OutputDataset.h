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
class NCFileCache;
}  // namespace mir::netcdf


namespace mir::netcdf {


class OutputDataset : public Dataset {
public:
    OutputDataset(const std::string&, NCFileCache&, int format = 0);
    ~OutputDataset() override;

    // -- Methods

    void merge(Dataset&);
    void save() const;

private:
    OutputDataset(const OutputDataset&);
    OutputDataset& operator=(const OutputDataset&);

    // -- Members

    int format_;
    NCFileCache& cache_;

    // - Methods

    // From Dataset

    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf

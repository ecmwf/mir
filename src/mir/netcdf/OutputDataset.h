// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

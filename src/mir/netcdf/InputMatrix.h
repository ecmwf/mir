// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Matrix.h"


namespace mir::netcdf {
class NCFile;
}  // namespace mir::netcdf


namespace mir::netcdf {


class InputMatrix : public Matrix {
public:
    InputMatrix(Type& type, int varid, const std::string& name, size_t size, NCFile& file);
    ~InputMatrix() override;

private:
    NCFile& file_;
    int varid_;

    // Methods
    void read(std::vector<double>&) const override;
    void read(std::vector<float>&) const override;
    void read(std::vector<long>&) const override;
    void read(std::vector<short>&) const override;
    void read(std::vector<unsigned char>&) const override;
    void read(std::vector<long long>&) const override;

    void read(std::vector<double>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const override;
    void read(std::vector<float>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const override;
    void read(std::vector<long>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const override;
    void read(std::vector<short>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const override;
    void read(std::vector<unsigned char>&, const std::vector<size_t>& start,
              const std::vector<size_t>& count) const override;
    void read(std::vector<long long>&, const std::vector<size_t>& start,
              const std::vector<size_t>& count) const override;

    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf

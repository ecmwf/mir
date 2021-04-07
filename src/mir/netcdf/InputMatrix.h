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

#include "mir/netcdf/Matrix.h"


namespace mir {
namespace netcdf {
class NCFile;
}
}  // namespace mir


namespace mir {
namespace netcdf {


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


}  // namespace netcdf
}  // namespace mir

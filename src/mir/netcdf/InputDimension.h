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

#include "mir/netcdf/Dimension.h"


namespace mir::netcdf {


class InputDimension : public Dimension {
public:
    InputDimension(Dataset& owner, const std::string& name, int id, size_t len);

    InputDimension(const InputDimension&) = delete;
    InputDimension(InputDimension&&)      = delete;

    ~InputDimension() override;

    void operator=(const InputDimension&) = delete;
    void operator=(InputDimension&&)      = delete;

private:
    // Members

    int id_;

    // -- Methods

    // From Dimension
    void print(std::ostream&) const override;
    void clone(Dataset& owner) const override;
    int id() const override;
    void realDimensions(std::vector<size_t>& dims) const override;
};


}  // namespace mir::netcdf

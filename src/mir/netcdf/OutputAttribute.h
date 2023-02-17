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

#include "mir/netcdf/Attribute.h"


namespace mir::netcdf {


class OutputAttribute : public Attribute {
public:
    OutputAttribute(Endowed& owner, const std::string& name, Value* value);
    ~OutputAttribute() override;

private:
    // -- Members

    bool valid_;

    // From Atttribute
    void create(int nc) const override;
    void print(std::ostream&) const override;
    void clone(Endowed& owner) const override;
    void merge(const Attribute&) override;
    void invalidate() override;
};


}  // namespace mir::netcdf

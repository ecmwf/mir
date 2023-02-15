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


class InputAttribute : public Attribute {
public:
    InputAttribute(Endowed& owner, const std::string& name, Value* value);
    ~InputAttribute() override;

private:
    // From Atttribute
    void print(std::ostream&) const override;
    void clone(Endowed& owner) const override;
};


}  // namespace mir::netcdf

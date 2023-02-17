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

#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


class InputVariable : public Variable {
public:
    InputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);
    ~InputVariable() override;

protected:
    // Members
    int id_;

    // -- Methods

    virtual Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                         const std::vector<Dimension*>& dimensions) const = 0;

    // From variable
    void print(std::ostream&) const override;
    Variable* clone(Dataset& owner) const override;

    // From Endowed

    int varid() const override;
};


}  // namespace mir::netcdf

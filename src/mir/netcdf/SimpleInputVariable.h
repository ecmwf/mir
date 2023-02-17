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

#include "mir/netcdf/InputVariable.h"


namespace mir::netcdf {


class SimpleInputVariable : public InputVariable {
public:
    SimpleInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);

    ~SimpleInputVariable() override;

private:
    std::vector<std::string> coordinates() const override;

    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;

    Variable* makeDataVariable() override;
    Variable* makeCoordinateVariable() override;
    Variable* makeCellMethodVariable() override;

    void print(std::ostream&) const override;
    void validate() const override;
    bool identified() const override;

    Variable* addMissingCoordinates() override;
};


}  // namespace mir::netcdf

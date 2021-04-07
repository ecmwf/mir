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


namespace mir {
namespace netcdf {


class DataInputVariable : public InputVariable {
public:
    DataInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);
    ~DataInputVariable() override;

private:
    std::vector<const Variable*> coordinates_;
    mutable std::string ncname_;

    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;
    void print(std::ostream&) const override;
    const std::string& ncname() const override;
    void collectField(std::vector<Field*>&) const override;

    size_t count2DValues() const override;
    void get2DValues(MIRValuesVector&, size_t i) const override;

    // Used during identtification
    void addCoordinateVariable(const Variable*) override;
    Variable* addMissingCoordinates() override;
    const char* kind() const override;
    void dumpAttributes(std::ostream&, const char* prefix) const override;

    std::vector<std::string> coordinates() const override;
};


}  // namespace netcdf
}  // namespace mir

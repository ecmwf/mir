// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/InputVariable.h"


namespace mir::netcdf {


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

    // Used during identification
    void addCoordinateVariable(const Variable*) override;
    Variable* addMissingCoordinates() override;
    const char* kind() const override;
    void dumpAttributes(std::ostream&, const char* prefix) const override;

    std::vector<std::string> coordinates() const override;
};


}  // namespace mir::netcdf

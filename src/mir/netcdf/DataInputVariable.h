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


#ifndef mir_netcdf_DataInputVariable_h
#define mir_netcdf_DataInputVariable_h

#include "mir/netcdf/InputVariable.h"


namespace mir {
namespace netcdf {


class DataInputVariable : public InputVariable {
public:
    DataInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);
    virtual ~DataInputVariable() override;

private:
    std::vector<const Variable*> coordinates_;
    mutable std::string ncname_;

    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;
    virtual void print(std::ostream&) const override;
    virtual const std::string& ncname() const override;
    virtual void collectField(std::vector<Field*>&) const override;

    virtual size_t count2DValues() const override;
    virtual void get2DValues(MIRValuesVector&, size_t i) const override;

    // Used during identtification
    virtual void addCoordinateVariable(const Variable*) override;
    virtual Variable* addMissingCoordinates() override;
    const char* kind() const override;
    void dumpAttributes(std::ostream& s, const char* prefix) const override;

    virtual std::vector<std::string> coordinates() const override;
};


}  // namespace netcdf
}  // namespace mir


#endif

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


#ifndef mir_netcdf_SimpleInputVariable_h
#define mir_netcdf_SimpleInputVariable_h

#include "mir/netcdf/InputVariable.h"


namespace mir {
namespace netcdf {


class SimpleInputVariable : public InputVariable {
public:
    SimpleInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);

    virtual ~SimpleInputVariable() override;

private:
    std::vector<std::string> coordinates() const override;

    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;

    virtual Variable* makeDataVariable() override;
    virtual Variable* makeCoordinateVariable() override;
    virtual Variable* makeCellMethodVariable() override;

    virtual void print(std::ostream&) const override;
    virtual void validate() const override;
    virtual bool identified() const override;

    Variable* addMissingCoordinates() override;
};


}  // namespace netcdf
}  // namespace mir


#endif

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


#ifndef mir_netcdf_ScalarCoordinateInputVariable_h
#define mir_netcdf_ScalarCoordinateInputVariable_h

#include "mir/netcdf/InputVariable.h"


namespace mir {
namespace netcdf {


class ScalarCoordinateInputVariable : public InputVariable {
public:
    ScalarCoordinateInputVariable(Dataset& owner, const std::string& name, int id,
                                  const std::vector<Dimension*>& dimensions);
    virtual ~ScalarCoordinateInputVariable() override;

private:
    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;
    virtual Dimension* getVirtualDimension() override;
    virtual Variable* makeCoordinateVariable() override;
    virtual Variable* makeScalarCoordinateVariable() override;
    virtual void print(std::ostream&) const override;
};


}  // namespace netcdf
}  // namespace mir


#endif

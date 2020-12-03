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


#ifndef mir_netcdf_ScalarCoordinateOutputVariable_h
#define mir_netcdf_ScalarCoordinateOutputVariable_h

#include "mir/netcdf/OutputVariable.h"


namespace mir {
namespace netcdf {


class ScalarCoordinateOutputVariable : public OutputVariable {
public:
    ScalarCoordinateOutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions);
    virtual ~ScalarCoordinateOutputVariable() override;

private:
    virtual void merge(const Variable&, MergePlan& plan) override;
    virtual void print(std::ostream&) const override;
};


}  // namespace netcdf
}  // namespace mir


#endif

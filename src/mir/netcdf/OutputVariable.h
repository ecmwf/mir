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


#ifndef mir_netcdf_OutputVariable_h
#define mir_netcdf_OutputVariable_h

#include "mir/netcdf/Variable.h"


namespace mir {
namespace netcdf {


class OutputVariable : public Variable {
public:
    OutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions);
    virtual ~OutputVariable() override;

private:
    // -- Members

    mutable int id_;
    mutable bool created_;

    // From variable

    virtual void print(std::ostream&) const override;
    virtual void create(int nc) const override;
    virtual void save(int nc) const override;

    // From Endowed

    virtual int varid() const override;
};


}  // namespace netcdf
}  // namespace mir


#endif

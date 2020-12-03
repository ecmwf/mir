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


#ifndef mir_netcdf_OutputAttribute_h
#define mir_netcdf_OutputAttribute_h

#include "mir/netcdf/Attribute.h"


namespace mir {
namespace netcdf {


class OutputAttribute : public Attribute {
public:
    OutputAttribute(Endowed& owner, const std::string& name, Value* value);
    virtual ~OutputAttribute() override;

private:
    // -- Members

    bool valid_;

    // From Atttribute
    virtual void create(int nc) const override;
    virtual void print(std::ostream& out) const override;
    virtual void clone(Endowed& owner) const override;
    virtual void merge(const Attribute&) override;
    virtual void invalidate() override;
};


}  // namespace netcdf
}  // namespace mir


#endif

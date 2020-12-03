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


#ifndef mir_netcdf_OutputDimension_h
#define mir_netcdf_OutputDimension_h

#include "mir/netcdf/Dimension.h"


namespace mir {
namespace netcdf {


class OutputDimension : public Dimension {
public:
    OutputDimension(Dataset& owner, const std::string& name, size_t len);
    virtual ~OutputDimension() override;

private:
    mutable int id_;
    mutable bool created_;
    bool grown_;

    // -- Methods

    // From Dimension
    virtual void print(std::ostream&) const override;
    virtual void create(int nc) const override;
    virtual int id() const override;
    virtual void grow(size_t) override;
};


}  // namespace netcdf
}  // namespace mir


#endif

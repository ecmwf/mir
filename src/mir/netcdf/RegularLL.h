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


#ifndef mir_netcdf_RegularLL_h
#define mir_netcdf_RegularLL_h

#include "mir/netcdf/GridSpec.h"


namespace mir {
namespace netcdf {


class RegularLL : public GridSpec {
public:
    RegularLL(const Variable&, double north, double south, double south_north_increment, double west, double east,
              double west_east_increment);

    virtual ~RegularLL();

    // -- Methods

    static GridSpec* guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes);


protected:
    // -- Members

    double north_;
    double south_;
    double south_north_increments_;

    double west_;
    double east_;
    double west_east_increment_;

    size_t ni_;
    size_t nj_;

    bool jScansPositively_;

private:
    RegularLL(const RegularLL&);
    RegularLL& operator=(const RegularLL&);

    // - Methods

    virtual void print(std::ostream& s) const;

    // From GridSpec
    virtual bool has(const std::string& name) const;
    virtual bool get(const std::string&, long&) const;
    virtual bool get(const std::string&, std::string&) const;
    virtual bool get(const std::string& name, double& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;
    virtual void reorder(MIRValuesVector& values) const;
};


}  // namespace netcdf
}  // namespace mir


#endif

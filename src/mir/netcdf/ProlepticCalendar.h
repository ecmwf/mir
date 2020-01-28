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


#ifndef mir_netcdf_GregorianDateCodec
#define mir_netcdf_GregorianDateCodec

#include "eckit/types/DateTime.h"
#include "mir/netcdf/Calendar.h"

namespace mir {
namespace netcdf {


class ProlepticCalendar : public Calendar {
public:
    ProlepticCalendar(const Variable& variable);
    virtual ~ProlepticCalendar();

private:
    // -- Methods

    virtual void print(std::ostream& s) const;
};

}  // namespace netcdf
}  // namespace mir
#endif

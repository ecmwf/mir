/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#ifndef mir_netcdf_Calendar
#define mir_netcdf_Calendar

#include "mir/netcdf/Codec.h"
#include "eckit/types/DateTime.h"

namespace mir {
namespace netcdf {

class Variable;


class Calendar : public Codec
{
public:

    Calendar();
    virtual ~Calendar();

private:

    virtual bool timeAxis() const;

};

}
}
#endif

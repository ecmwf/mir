// File Grib.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Grib_H
#define Grib_H

#include "eckit/exception/Exceptions.h"

#include <grib_api.h>

static void grib_call(int e, const char *call) {
    if (e) {
        eckit::StrStream os;
        os << call << ": " << grib_get_error_message(e) << eckit::StrStream::ends;
        throw eckit::SeriousBug(std::string(os));
    }
}

#define GRIB_CALL(a) grib_call(a, #a)
#define GRIB_ERROR(a, b) grib_call(a, b)

struct grib_spec
{
    grib_util_grid_spec grid;
    grib_util_packing_spec packing;
};


#endif

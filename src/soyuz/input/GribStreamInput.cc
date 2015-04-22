/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/exception/Exceptions.h"
#include "eckit/io/BufferedHandle.h"

#include "soyuz/data/MIRField.h"
#include "soyuz/util/Grib.h"

#include "soyuz/input/GribStreamInput.h"


namespace mir {
namespace input {
namespace {


const size_t SIXTY_FOUR_MB = 64 * 1024 * 1024;


static long readcb(void *data, void *buffer, long len) {
    eckit::DataHandle *handle = reinterpret_cast<eckit::DataHandle *>(data);
    return handle->read(buffer, len);
}


}  // (anonymous namespace)


GribStreamInput::GribStreamInput():
    buffer_(SIXTY_FOUR_MB) {
}


GribStreamInput::~GribStreamInput() {
}


bool GribStreamInput::next() {
    handle(0);

    size_t len = buffer_.size();
    int e    = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);

    if (e == GRIB_SUCCESS)  {
        ASSERT(handle(grib_handle_new_from_message(0, buffer_, len)));
        return true;
    }

    if (e == GRIB_END_OF_FILE) return false;

    GRIB_ERROR(e, "grib_handle_new_from_message");
    // Not reached
    return false;
}


}  // namespace input
}  // namespace mir


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

#include "mir/data/MIRField.h"
#include "mir/util/Grib.h"

#include "mir/input/GribStreamInput.h"


namespace mir {
namespace input {
namespace {


const size_t SIXTY_FOUR_MB = 64 * 1024 * 1024;


static long readcb(void *data, void *buffer, long len) {
    eckit::DataHandle *handle = reinterpret_cast<eckit::DataHandle *>(data);
    return handle->read(buffer, len);
}


}  // (anonymous namespace)


GribStreamInput::GribStreamInput(size_t skip, size_t step):
    skip_(skip),
    step_(step),
    first_(true),
    buffer_(SIXTY_FOUR_MB) {
    ASSERT(step_ > 0);
}


GribStreamInput::~GribStreamInput() {
}


bool GribStreamInput::next() {

    handle(0);

    // Skip a few message if needed
    size_t advance = step_ - 1;

    if (first_) {
        first_ = false;
        advance = skip_;
    }

    for (size_t i = 0; i < advance; i++) {
        size_t len = buffer_.size();
        int e  = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);
        if (e == GRIB_END_OF_FILE) {
            return false;

        }
        if (e != GRIB_SUCCESS) {
            GRIB_ERROR(e, "wmo_read_any_from_stream");
        }
    }

    size_t len = buffer_.size();
    int e    = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);

    if (e == GRIB_SUCCESS)  {
        ASSERT(handle(grib_handle_new_from_message(0, buffer_, len)));
        return true;
    }

    if (e == GRIB_END_OF_FILE) return false;

    GRIB_ERROR(e, "wmo_read_any_from_stream");
    // Not reached
    return false;
}


}  // namespace input
}  // namespace mir


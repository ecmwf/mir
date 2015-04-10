// File GribStreamInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/input/GribStreamInput.h"
#include "soyuz/data/MIRField.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/BufferedHandle.h"

#include "soyuz/util/Grib.h"


const size_t SIXTY_FOUR_MB = 64 * 1024 * 1024;

static long readcb(void *data, void *buffer, long len) {
    eckit::DataHandle *handle = reinterpret_cast<eckit::DataHandle *>(data);
    return handle->read(buffer, len);
}


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



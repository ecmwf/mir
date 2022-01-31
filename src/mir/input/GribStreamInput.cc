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


#include "mir/input/GribStreamInput.h"

#include "eckit/config/Resource.h"
#include "eckit/io/DataHandle.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace input {


static size_t buffer_size() {
    constexpr size_t BUFFER_SIZE = 64 * 1024 * 1024;
    static size_t size           = eckit::Resource<size_t>("$MIR_GRIB_INPUT_BUFFER_SIZE", BUFFER_SIZE);
    return size;
}


static long readcb(void* data, void* buffer, long len) {
    auto* handle = reinterpret_cast<eckit::DataHandle*>(data);
    long l       = handle->read(buffer, len);
    // ecCodes only interprets a -1 as EOF
    return l == 0 ? -1 : l;
}


GribStreamInput::GribStreamInput(size_t skip, size_t step) :
    skip_(skip), step_(step), offset_(0), buffer_(buffer_size()), first_(true) {
    ASSERT(step_ > 0);
}


GribStreamInput::GribStreamInput(off_t offset) :
    skip_(0), step_(1), offset_(offset), buffer_(buffer_size()), first_(true) {
    ASSERT(step_ > 0);
}


GribStreamInput::GribStreamInput() : skip_(0), step_(1), offset_(0), buffer_(buffer_size()), first_(true) {
    ASSERT(step_ > 0);
}


GribStreamInput::~GribStreamInput() = default;


bool GribStreamInput::next() {

    handle(nullptr);

    // Skip a few message if needed
    size_t advance = step_ - 1;

    if (first_) {
        first_  = false;
        advance = skip_;

        if (offset_ != 0) {
            dataHandle().skip(offset_);
        }
    }

    for (size_t i = 0; i < advance; i++) {
        size_t len = buffer_.size();
        int e      = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);
        if (e == CODES_END_OF_FILE) {
            return false;
        }

        if (e == CODES_BUFFER_TOO_SMALL) {
            Log::debug() << "GribStreamInput::next() message is " << len << " bytes (" << Log::Bytes(len) << ")"
                         << std::endl;
            GRIB_ERROR(e, "wmo_read_any_from_stream");
        }

        if (e != CODES_SUCCESS) {
            GRIB_ERROR(e, "wmo_read_any_from_stream");
        }
    }

    size_t len = buffer_.size();
    int e      = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);

    if (e == CODES_SUCCESS) {
        ASSERT(handle(codes_handle_new_from_message(nullptr, buffer_, len)));
        return true;
    }

    if (e == CODES_END_OF_FILE) {
        return false;
    }


    if (e == CODES_BUFFER_TOO_SMALL) {
        Log::debug() << "GribStreamInput::next() message is " << len << " bytes (" << Log::Bytes(len) << ")"
                     << std::endl;
        Log::debug() << "Buffer size is " << buffer_.size() << " bytes (" << Log::Bytes(buffer_.size())
                     << "), rerun with:" << std::endl;
        Log::debug() << "env MIR_GRIB_INPUT_BUFFER_SIZE=" << len << std::endl;
        GRIB_ERROR(e, "wmo_read_any_from_stream");
    }

    GRIB_ERROR(e, "wmo_read_any_from_stream");
    // Not reached
    return false;
}


}  // namespace input
}  // namespace mir

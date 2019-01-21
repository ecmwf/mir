/*
 * (C) Copyright 1996- ECMWF.
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
#include "eckit/config/Resource.h"
#include "eckit/log/Bytes.h"
#include "mir/config/LibMir.h"

#include "mir/data/MIRField.h"
#include "mir/util/Grib.h"

#include "mir/input/GribStreamInput.h"


namespace mir {
namespace input {
namespace {


static size_t buffer_size() {
    static size_t size = eckit::Resource<size_t>("$MIR_GRIB_INPUT_BUFFER_SIZE", 64 * 1024 * 1024);
    return size;
}

static long readcb(void *data, void *buffer, long len) {
    eckit::DataHandle *handle = reinterpret_cast<eckit::DataHandle *>(data);
    const long l = handle->read(buffer, len);
    // ecCodes only interprets a -1 as EOF
    return l == 0 ? -1 : l;
}


}  // (anonymous namespace)


GribStreamInput::GribStreamInput(size_t skip, size_t step):
    skip_(skip),
    step_(step),
    offset_(0),
    first_(true),
    buffer_(buffer_size()) {
    ASSERT(step_ > 0);
}


GribStreamInput::GribStreamInput(off_t offset):
    skip_(0),
    step_(1),
    offset_(offset),
    first_(true),
    buffer_(buffer_size()) {
    ASSERT(step_ > 0);
}


GribStreamInput::GribStreamInput():
    skip_(0),
    step_(1),
    offset_(0),
    first_(true),
    buffer_(buffer_size()) {
    ASSERT(step_ > 0);
}

GribStreamInput::~GribStreamInput() = default;


bool GribStreamInput::next() {

    handle(nullptr);

    // Skip a few message if needed
    size_t advance = step_ - 1;

    if (first_) {
        first_ = false;
        advance = skip_;

        if(offset_) {
            dataHandle().skip(offset_);
        }
    }

    for (size_t i = 0; i < advance; i++) {
        size_t len = buffer_.size();
        int e  = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);
        if (e == GRIB_END_OF_FILE) {
            return false;

        }

        if (e == GRIB_BUFFER_TOO_SMALL) {
            eckit::Log::debug<LibMir>() << "GribStreamInput::next() message is " << len << " bytes (" << eckit::Bytes(len) << ")" << std::endl;
            GRIB_ERROR(e, "wmo_read_any_from_stream");
        }

        if (e != GRIB_SUCCESS) {
            GRIB_ERROR(e, "wmo_read_any_from_stream");
        }
    }

    size_t len = buffer_.size();
    int e    = wmo_read_any_from_stream(&dataHandle(), &readcb, buffer_, &len);

    if (e == GRIB_SUCCESS)  {
        ASSERT(handle(grib_handle_new_from_message(nullptr, buffer_, len)));
        return true;
    }

    if (e == GRIB_END_OF_FILE) {
        return false;
    }


    if (e == GRIB_BUFFER_TOO_SMALL) {
        eckit::Log::debug<LibMir>() << "GribStreamInput::next() message is " << len << " bytes (" << eckit::Bytes(len) << ")" << std::endl;
        eckit::Log::debug<LibMir>() << "Buffer size is " << buffer_.size() << " bytes (" << eckit::Bytes(buffer_.size()) << "), rerun with:" << std::endl;
        eckit::Log::debug<LibMir>() << "env MIR_GRIB_INPUT_BUFFER_SIZE=" << len << std::endl;
        GRIB_ERROR(e, "wmo_read_any_from_stream");
    }

    GRIB_ERROR(e, "wmo_read_any_from_stream");
    // Not reached
    return false;
}


}  // namespace input
}  // namespace mir


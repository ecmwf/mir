// File MIRInput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "MIRInput.h"
#include "eckit/exception/Exceptions.h"
#include "Grib.h"

MIRInput::MIRInput() {
}

MIRInput::~MIRInput() {
}

grib_handle *MIRInput::gribHandle() const {
    static grib_handle* handle = 0;
    if (!handle) {
        handle = grib_handle_new_from_samples(0, "GRIB1");
        ASSERT(handle);
    }
    return handle;
}

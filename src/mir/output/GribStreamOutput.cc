// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/io/DataHandle.h"

#include "mir/output/GribStreamOutput.h"


namespace mir::output {


GribStreamOutput::GribStreamOutput() = default;


GribStreamOutput::~GribStreamOutput() = default;


void GribStreamOutput::out(const void* message, size_t length, bool /*interpolated*/) {
    dataHandle().write(message, long(length));
}


}  // namespace mir::output

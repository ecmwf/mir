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


#include "eckit/io/DataHandle.h"

#include "mir/output/GribStreamOutput.h"


namespace mir::output {


GribStreamOutput::GribStreamOutput() = default;


GribStreamOutput::~GribStreamOutput() = default;


void GribStreamOutput::out(const void* message, size_t length, bool /*interpolated*/) {
    dataHandle().write(message, long(length));
}


}  // namespace mir::output

// File GribStreamOutput.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/output/GribStreamOutput.h"
#include "eckit/io/DataHandle.h"


GribStreamOutput::GribStreamOutput() {
}

GribStreamOutput::~GribStreamOutput() {
}

void GribStreamOutput::out(const void* message, size_t length, bool iterpolated)
{
    dataHandle().write(message, length);
}


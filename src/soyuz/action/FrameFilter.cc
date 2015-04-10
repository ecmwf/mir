// File FrameFilter.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/action/FrameFilter.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/util/MIRField.h"
#include "soyuz/repres/Representation.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/StdFile.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"


FrameFilter::FrameFilter(const MIRParametrisation &parametrisation):
    Action(parametrisation),
    size_(0) {

    std::string size;
    ASSERT(parametrisation.get("frame", size));
    size_ = eckit::Translator<std::string, long>()(size);
}

FrameFilter::~FrameFilter() {
}

void FrameFilter::print(std::ostream &out) const {
    out << "FrameFilter[size=" << size_ << "]";
}

void FrameFilter::execute(MIRField &field) const {

    double missingValue = field.missingValue();
    std::vector<double> &values = field.values();

    const Representation *representation = field.representation();
    size_t count = representation->frame(values, size_, missingValue);

    if (count) {
        field.hasMissing(true);
    } else {
        eckit::Log::warning() << "Frame " << size_ << " has no effect" << std::endl;
    }

}

static ActionBuilder<FrameFilter> bitmapFilter("filter.frame");

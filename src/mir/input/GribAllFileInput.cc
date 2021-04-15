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


#include "mir/input/GribAllFileInput.h"

#include <ostream>

#include "eckit/io/Buffer.h"
#include "eckit/io/StdFile.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir {
namespace input {


GribAllFileInput::GribAllFileInput(const std::string& path) : path_(path), count_(0) {

    eckit::AutoStdFile f(path);
    eckit::Buffer buffer(64 * 1024 * 1024);

    for (;;) {
        size_t len = buffer.size();
        off_t here;
        SYSCALL(here = ::ftello(f));
        int e = wmo_read_any_from_file(f, buffer, &len);
        if (e == CODES_END_OF_FILE) {
            break;
        }

        if (e == CODES_BUFFER_TOO_SMALL) {
            GRIB_ERROR(e, "wmo_read_any_from_file");
        }

        if (e != CODES_SUCCESS) {
            GRIB_ERROR(e, "wmo_read_any_from_file");
        }

        inputs_.push_back(new GribFileInput(path, here));
    }
}


GribAllFileInput::~GribAllFileInput() {
    for (auto& j : inputs_) {
        delete j;
    }
}


const param::MIRParametrisation& GribAllFileInput::parametrisation(size_t which) const {
    ASSERT(which < inputs_.size());
    return inputs_[which]->parametrisation();
}


grib_handle* GribAllFileInput::gribHandle(size_t which) const {
    ASSERT(which < inputs_.size());
    return inputs_[which]->gribHandle();
}


data::MIRField GribAllFileInput::field() const {
    ASSERT(inputs_.size());
    data::MIRField f(inputs_[0]->field());
    ASSERT(f.dimensions() == 1);
    for (size_t i = 1; i < inputs_.size(); i++) {
        data::MIRField g(inputs_[i]->field());
        ASSERT(g.dimensions() == 1);
        f.update(g.direct(0), i);
    }

    return f;
}


bool GribAllFileInput::next() {
    if (count_ == 0) {
        for (auto& j : inputs_) {
            // Log::info() << *(*j) << std::endl;
            ASSERT(j->next());
        }
        return true;
    }
    return false;
}


bool GribAllFileInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const GribAllFileInput*>(&other);
    return (o != nullptr) && path_ == o->path_;
}


void GribAllFileInput::print(std::ostream& out) const {
    out << "GribAllFileInput[" << path_ << ",size=" << inputs_.size() << "]";
}


size_t GribAllFileInput::dimensions() const {
    return inputs_.size();
}


}  // namespace input
}  // namespace mir

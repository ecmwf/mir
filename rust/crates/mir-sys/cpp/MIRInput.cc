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


#include "MIRInput.h"

#include <string>

#include "eckit/filesystem/PathName.h"

#include "mir/input/GribDataHandleInput.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/GribMemoryInput.h"
#include "mir/input/GridSpecInput.h"
#include "mir/input/MultiDimensionalGribFileInput.h"
#include "mir/input/RawInput.h"


namespace mir_bridge {


bool MIRInput::next() {
    return input_->next();
}


size_t MIRInput::dimensions() const {
    return input_->dimensions();
}


std::unique_ptr<MIRInput> MIRInput::from_data_handle(eckit_bridge::DataHandleWrapper& handle) {
    auto wrapper    = std::make_unique<MIRInput>();
    wrapper->input_ = std::make_unique<mir::input::GribDataHandleInput>(handle.inner());
    return wrapper;
}


std::unique_ptr<MIRInput> MIRInput::from_grib_file(rust::Str path) {
    auto wrapper    = std::make_unique<MIRInput>();
    wrapper->input_ = std::make_unique<mir::input::GribFileInput>(eckit::PathName(std::string(path)));
    return wrapper;
}


std::unique_ptr<MIRInput> MIRInput::from_grib_memory(rust::Slice<const uint8_t> message) {
    auto wrapper      = std::make_unique<MIRInput>();
    wrapper->message_ = std::vector<unsigned char>(message.begin(), message.end());
    wrapper->input_ = std::make_unique<mir::input::GribMemoryInput>(wrapper->message_.data(), wrapper->message_.size());
    return wrapper;
}


std::unique_ptr<MIRInput> MIRInput::from_multi_dimensional_grib_file(rust::Str path, size_t dimensions, size_t skip) {
    auto wrapper    = std::make_unique<MIRInput>();
    wrapper->input_ = std::make_unique<mir::input::MultiDimensionalGribFileInput>(eckit::PathName(std::string(path)),
                                                                                  dimensions, skip);
    return wrapper;
}


std::unique_ptr<MIRInput> MIRInput::from_gridspec(rust::Str gridspec, bool gridded) {
    auto wrapper    = std::make_unique<MIRInput>();
    wrapper->input_ = std::make_unique<mir::input::GridSpecInput>(std::string(gridspec), gridded);
    return wrapper;
}


std::unique_ptr<MIRInput> MIRInput::from_raw(rust::Slice<const double> values, const Parametrisation& metadata) {
    auto wrapper     = std::make_unique<MIRInput>();
    wrapper->values_ = std::vector<double>(values.begin(), values.end());

    // Copied before the input is built: `RawInput`'s constructor reads
    // `dimensions` off the metadata it is handed.
    metadata.copyValuesTo(wrapper->metadata_);

    wrapper->input_ =
        std::make_unique<mir::input::RawInput>(wrapper->values_.data(), wrapper->values_.size(), wrapper->metadata_);
    return wrapper;
}


}  // namespace mir_bridge

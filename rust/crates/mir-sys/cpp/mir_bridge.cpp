// mir C++ bridge implementation
#include "mir_bridge.h"
#include "mir-sys/src/lib.rs.h"

#include <sstream>
#include <string>
#include <vector>

namespace mir_bridge {

// ==================== RustGribOutput ====================

/// GribOutput subclass that calls back into Rust via the OutputBox.
class RustGribOutput : public mir::output::GribOutput {
    rust::Box<OutputBox> rust_;

public:
    explicit RustGribOutput(rust::Box<OutputBox> output) : rust_(std::move(output)) {}

    bool sameAs(const MIROutput&) const override { return false; }

    void print(std::ostream& s) const override { s << "RustGribOutput[]"; }

    void out(const void* message, size_t length, bool) override {
        auto slice = rust::Slice<const uint8_t>(static_cast<const uint8_t*>(message), length);
        invoke_output(*rust_, slice);
    }
};

// ==================== GribDataHandleInput ====================

GribInputWrapper::GribInputWrapper(eckit_bridge::DataHandleWrapper& handle) :
    input_(std::make_unique<mir::input::GribDataHandleInput>(handle.inner())) {}

bool GribInputWrapper::next() {
    return input_->next();
}

std::unique_ptr<GribInputWrapper> grib_input_create(eckit_bridge::DataHandleWrapper& handle) {
    return std::make_unique<GribInputWrapper>(handle);
}

// ==================== VectorInput ====================

VectorInputWrapper::VectorInputWrapper(GribInputWrapper& component1, GribInputWrapper& component2) :
    input_(std::make_unique<mir::input::VectorInput>(component1.as_mir_input(), component2.as_mir_input())) {}

std::unique_ptr<VectorInputWrapper> vector_input_create(GribInputWrapper& component1, GribInputWrapper& component2) {
    return std::make_unique<VectorInputWrapper>(component1, component2);
}

// ==================== MIRJob ====================

void MIRJobWrapper::set_string(rust::Str name, rust::Str value) {
    job_.set(std::string(name), std::string(value));
}

void MIRJobWrapper::set_double(rust::Str name, double value) {
    job_.set(std::string(name), value);
}

void MIRJobWrapper::set_long(rust::Str name, int64_t value) {
    job_.set(std::string(name), static_cast<long>(value));
}

void MIRJobWrapper::set_bool(rust::Str name, bool value) {
    job_.set(std::string(name), value);
}

void MIRJobWrapper::set_double_list(rust::Str name, rust::Slice<const double> values) {
    std::vector<double> vec(values.begin(), values.end());
    job_.set(std::string(name), vec);
}

void MIRJobWrapper::set_from_string(rust::Str args) {
    job_.set(std::string(args));
}

void MIRJobWrapper::execute(eckit_bridge::DataHandleWrapper& input, rust::Box<OutputBox> output) {
    mir::input::GribDataHandleInput mir_input(input.inner());
    RustGribOutput mir_output(std::move(output));
    job_.execute(mir_input, mir_output);
}

void MIRJobWrapper::execute_input(GribInputWrapper& input, rust::Box<OutputBox> output) {
    RustGribOutput mir_output(std::move(output));
    job_.execute(input.as_mir_input(), mir_output);
}

void MIRJobWrapper::execute_vector(VectorInputWrapper& input, rust::Box<OutputBox> output) {
    RustGribOutput mir_output(std::move(output));
    job_.execute(input.as_mir_input(), mir_output);
}

void MIRJobWrapper::clear(rust::Str name) {
    job_.clear(std::string(name));
}

void MIRJobWrapper::representation_from(GribInputWrapper& input) {
    job_.representationFrom(input.as_mir_input());
}

rust::String MIRJobWrapper::json_str() const {
    return rust::String(job_.json_str());
}

rust::String MIRJobWrapper::mir_tool_call() const {
    std::ostringstream oss;
    job_.mirToolCall(oss);
    return rust::String(oss.str());
}

std::unique_ptr<MIRJobWrapper> mir_job_create() {
    return std::make_unique<MIRJobWrapper>();
}

}  // namespace mir_bridge

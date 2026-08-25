#include "MIROutput.h"

#include <string>
#include <utility>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

#include "mir/output/EmptyOutput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/output/GribMemoryOutput.h"
#include "mir/output/GribOutput.h"
#include "mir/output/ResizableOutput.h"

#include "mir-sys/src/lib.rs.h"


namespace mir_bridge {


namespace {


/// GribOutput that hands each encoded message to a Rust closure.
class CallbackOutput final : public mir::output::GribOutput {
    rust::Box<OutputBox> rust_;

public:
    explicit CallbackOutput(rust::Box<OutputBox> output) : rust_(std::move(output)) {}

    bool sameAs(const MIROutput& /*other*/) const override { return false; }

    void print(std::ostream& s) const override { s << "CallbackOutput[]"; }

private:
    void out(const void* message, size_t length, bool /*interpolated*/) override {
        invoke_output(*rust_, rust::Slice<const uint8_t>(static_cast<const uint8_t*>(message), length));
    }
};


/// Downcast `output` or throw: the readback accessors only apply to the
/// factory that produced the matching kind.
template <class T>
const T& as(const std::unique_ptr<mir::output::MIROutput>& output, const char* what) {
    const auto* cast = dynamic_cast<const T*>(output.get());
    if (cast == nullptr) {
        throw eckit::SeriousBug(std::string("MIROutput::") + what);
    }
    return *cast;
}


}  // namespace


rust::Slice<const double> MIROutput::values() const {
    as<mir::output::ResizableOutput>(output_, "values requires an output built by to_resizable");
    return {values_.data(), values_.size()};
}


rust::String MIROutput::metadata_json() const {
    as<mir::output::ResizableOutput>(output_, "metadata_json requires an output built by to_resizable");
    return metadata_.to_json();
}


rust::Slice<const uint8_t> MIROutput::message() const {
    const auto& grib = as<mir::output::GribMemoryOutput>(output_, "message requires an output built by to_grib_memory");
    return {message_.data(), grib.length()};
}


std::unique_ptr<MIROutput> MIROutput::to_callback(rust::Box<OutputBox> output) {
    auto wrapper     = std::make_unique<MIROutput>();
    wrapper->output_ = std::make_unique<CallbackOutput>(std::move(output));
    return wrapper;
}


std::unique_ptr<MIROutput> MIROutput::to_grib_file(rust::Str path, bool append) {
    auto wrapper     = std::make_unique<MIROutput>();
    wrapper->output_ = std::make_unique<mir::output::GribFileOutput>(eckit::PathName(std::string(path)), append);
    return wrapper;
}


std::unique_ptr<MIROutput> MIROutput::to_grib_memory(size_t capacity) {
    auto wrapper      = std::make_unique<MIROutput>();
    wrapper->message_ = std::vector<unsigned char>(capacity, 0);
    wrapper->output_ =
        std::make_unique<mir::output::GribMemoryOutput>(wrapper->message_.data(), wrapper->message_.size());
    return wrapper;
}


std::unique_ptr<MIROutput> MIROutput::to_resizable() {
    auto wrapper     = std::make_unique<MIROutput>();
    wrapper->output_ = std::make_unique<mir::output::ResizableOutput>(wrapper->values_, wrapper->metadata_);
    return wrapper;
}


std::unique_ptr<MIROutput> MIROutput::to_empty() {
    auto wrapper     = std::make_unique<MIROutput>();
    wrapper->output_ = std::make_unique<mir::output::EmptyOutput>();
    return wrapper;
}


}  // namespace mir_bridge

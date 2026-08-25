#include "Job.h"

#include <sstream>
#include <string>
#include <vector>

namespace mir_bridge {

//----------------------------------------------------------------------------------------------------------------------

void Job::set_str(rust::Str name, rust::Str value) {
    set(std::string(name), std::string(value));
}

void Job::set_f64(rust::Str name, double value) {
    set(std::string(name), value);
}

void Job::set_i64(rust::Str name, int64_t value) {
    set(std::string(name), static_cast<long long>(value));
}

void Job::set_bool(rust::Str name, bool value) {
    set(std::string(name), value);
}

void Job::set_f64_list(rust::Str name, rust::Slice<const double> values) {
    set(std::string(name), std::vector<double>(values.begin(), values.end()));
}

void Job::set_i64_list(rust::Str name, rust::Slice<const int64_t> values) {
    set(std::string(name), std::vector<long long>(values.begin(), values.end()));
}

void Job::set_str_list(rust::Str name, const rust::Vec<rust::String>& values) {
    std::vector<std::string> converted;
    converted.reserve(values.size());
    for (const auto& value : values) {
        converted.emplace_back(value);
    }
    set(std::string(name), converted);
}

void Job::set_from_string(rust::Str args) {
    set(std::string(args));
}

void Job::clear_key(rust::Str name) {
    clear(std::string(name));
}

void Job::representation_from(const MIRInput& input) {
    representationFrom(input.inner());
}

//----------------------------------------------------------------------------------------------------------------------

rust::String Job::to_json() const {
    return rust::String(json_str());
}

rust::String Job::mir_tool_call() const {
    std::ostringstream s;
    mirToolCall(s);
    return rust::String(s.str());
}

//----------------------------------------------------------------------------------------------------------------------

void Job::execute_one(MIRInput& input, MIROutput& output) const {
    execute(input.inner(), output.inner());
}

size_t Job::execute_all(MIRInput& input, MIROutput& output) const {
    size_t processed = 0;
    while (input.next()) {
        execute(input.inner(), output.inner());
        ++processed;
    }
    return processed;
}

std::unique_ptr<Job> Job::make() {
    return std::make_unique<Job>();
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mir_bridge

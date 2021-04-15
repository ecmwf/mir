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


#include "mir/param/SimpleParametrisation.h"

#include <algorithm>
#include <ios>
#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace param {


class Setting {
public:
    Setting()          = default;
    virtual ~Setting() = default;

    Setting(const Setting&) = delete;
    Setting& operator=(const Setting&) = delete;

    virtual void get(const std::string& name, std::string& value) const = 0;
    virtual void get(const std::string& name, bool& value) const        = 0;
    virtual void get(const std::string& name, int& value) const         = 0;
    virtual void get(const std::string& name, long& value) const        = 0;
    virtual void get(const std::string& name, size_t& value) const      = 0;
    virtual void get(const std::string& name, float& value) const       = 0;
    virtual void get(const std::string& name, double& value) const      = 0;

    virtual void get(const std::string& name, std::vector<int>& value) const         = 0;
    virtual void get(const std::string& name, std::vector<long>& value) const        = 0;
    virtual void get(const std::string& name, std::vector<size_t>& value) const      = 0;
    virtual void get(const std::string& name, std::vector<float>& value) const       = 0;
    virtual void get(const std::string& name, std::vector<double>& value) const      = 0;
    virtual void get(const std::string& name, std::vector<std::string>& value) const = 0;

    virtual bool match(const std::string& name, const MIRParametrisation&) const    = 0;
    virtual void copyValueTo(const std::string& name, SimpleParametrisation&) const = 0;

    virtual void print(std::ostream&) const = 0;
    virtual void json(eckit::JSON&) const   = 0;

    friend std::ostream& operator<<(std::ostream& s, const Setting& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Setting& p) {
        p.json(s);
        return s;
    }
};


template <class T>
const char* TNamed() {
    NOTIMP;
}


template <>
const char* TNamed<std::string>() {
    return "string";
}


template <>
const char* TNamed<bool>() {
    return "bool";
}


template <>
const char* TNamed<int>() {
    return "int";
}


template <>
const char* TNamed<long>() {
    return "long";
}


template <>
const char* TNamed<size_t>() {
    return "size_t";
}


template <>
const char* TNamed<float>() {
    return "float";
}


template <>
const char* TNamed<double>() {
    return "double";
}


template <>
const char* TNamed<std::vector<int>>() {
    return "vector<int>";
}


template <>
const char* TNamed<std::vector<long>>() {
    return "vector<long>";
}


template <>
const char* TNamed<std::vector<size_t>>() {
    return "vector<size_t>";
}


template <>
const char* TNamed<std::vector<float>>() {
    return "vector<float>";
}


template <>
const char* TNamed<std::vector<double>>() {
    return "vector<double>";
}


template <>
const char* TNamed<std::vector<std::string>>() {
    return "vector<string>";
}


template <class T>
static void conversion_warning(const char* /*from*/, const char* /*to*/, const std::string& /*name*/,
                               const T& /*value*/) {
    // Log::warning() << "   +++ WARNING: Converting " << value << " from " << from << " to " << to << "
    // (requesting " << name << ")" << std::endl;
}


template <class T>
class TSettings : public Setting {
    const T value_;

public:
    TSettings(const T& value) : value_(value) {}

    void get(const std::string& name, std::string&) const override {
        throw exception::CannotConvert(TNamed<T>(), "string", name, value_);
    }

    void get(const std::string& name, bool&) const override {
        throw exception::CannotConvert(TNamed<T>(), "bool", name, value_);
    }

    void get(const std::string& name, int&) const override {
        throw exception::CannotConvert(TNamed<T>(), "int", name, value_);
    }

    void get(const std::string& name, long&) const override {
        throw exception::CannotConvert(TNamed<T>(), "long", name, value_);
    }

    void get(const std::string& name, size_t&) const override {
        throw exception::CannotConvert(TNamed<T>(), "size_t", name, value_);
    }

    void get(const std::string& name, float&) const override {
        throw exception::CannotConvert(TNamed<T>(), "float", name, value_);
    }

    void get(const std::string& name, double&) const override {
        throw exception::CannotConvert(TNamed<T>(), "double", name, value_);
    }

    void get(const std::string& name, std::vector<int>&) const override {
        throw exception::CannotConvert(TNamed<T>(), "vector<int>", name, value_);
    }

    void get(const std::string& name, std::vector<long>&) const override {
        throw exception::CannotConvert(TNamed<T>(), "vector<long>", name, value_);
    }

    void get(const std::string& name, std::vector<size_t>&) const override {
        throw exception::CannotConvert(TNamed<T>(), "vector<size_t>", name, value_);
    }

    void get(const std::string& name, std::vector<float>&) const override {
        throw exception::CannotConvert(TNamed<T>(), "vector<float>", name, value_);
    }

    void get(const std::string& name, std::vector<double>&) const override {
        throw exception::CannotConvert(TNamed<T>(), "vector<double>", name, value_);
    }

    void get(const std::string& name, std::vector<std::string>&) const override {
        throw exception::CannotConvert(TNamed<T>(), "vector<string>", name, value_);
    }

    bool match(const std::string& name, const MIRParametrisation& other) const override {
        T value;
        if (other.get(name, value)) {
            return value_ == value;
        }
        return false;
    }

    void copyValueTo(const std::string& name, SimpleParametrisation& param) const override { param.set(name, value_); }

    void print(std::ostream& out) const override { out << value_; }

    void json(eckit::JSON& out) const override { out << value_; }
};


template <class T>
static void _put(std::ostream& out, const std::vector<T>& v) {
    const char* sep   = "";
    const char* comma = ", ";
    if (eckit::format(out) == Log::applicationFormat) {
        comma = "/";
    }
    for (size_t i = 0; i < v.size(); i++) {
        out << sep << v[i];
        sep = comma;
    }
}


template <>
void TSettings<std::vector<long>>::print(std::ostream& out) const {
    _put(out, value_);
}


template <>
void TSettings<std::vector<double>>::print(std::ostream& out) const {
    _put(out, value_);
}


template <class T>
bool any_of(const std::vector<T>& values, const T& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

template <>
bool TSettings<std::vector<std::string>>::match(const std::string& name, const MIRParametrisation& other) const {
    // if any of "these values" matches "other value"
    std::string value;
    return other.get(name, value) && any_of(value_, value);
}


// implement conversion as needed
template <>
void TSettings<bool>::get(const std::string&, std::string& value) const {
    std::ostringstream ss;
    ss << std::boolalpha << value_;
    value = ss.str();
}


template <>
void TSettings<bool>::get(const std::string&, bool& value) const {
    value = value_;
}


template <>
void TSettings<long>::get(const std::string&, long& value) const {
    value = value_;
}


template <>
void TSettings<long>::get(const std::string&, int& value) const {
    value = int(value_);
}


template <>
void TSettings<int>::get(const std::string&, long& value) const {
    value = value_;
}


template <>
void TSettings<double>::get(const std::string&, double& value) const {
    value = value_;
}


template <>
void TSettings<std::string>::get(const std::string&, std::string& value) const {
    value = value_;
}


template <>
void TSettings<std::string>::get(const std::string& name, bool& value) const {
    conversion_warning("string", "bool", name, value_);
    eckit::Translator<std::string, long> translate;
    value = translate(value_) != 0;
}


template <>
void TSettings<std::string>::get(const std::string& name, int& value) const {
    conversion_warning("string", "int", name, value_);
    eckit::Translator<std::string, int> translate;
    value = translate(value_);
}


template <>
void TSettings<std::string>::get(const std::string& name, long& value) const {
    conversion_warning("string", "long", name, value_);
    eckit::Translator<std::string, long> translate;
    value = translate(value_);
}


template <>
void TSettings<std::string>::get(const std::string& name, size_t& value) const {
    conversion_warning("string", "size_t", name, value_);
    eckit::Translator<std::string, size_t> translate;
    value = translate(value_);
}


template <>
void TSettings<std::string>::get(const std::string& name, float& value) const {
    conversion_warning("string", "float", name, value_);
    eckit::Translator<std::string, float> translate;
    value = translate(value_);
}


template <>
void TSettings<std::string>::get(const std::string& name, double& value) const {
    conversion_warning("string", "double", name, value_);
    eckit::Translator<std::string, double> translate;
    value = translate(value_);
}


template <>
void TSettings<std::string>::get(const std::string& name, std::vector<double>& value) const {
    conversion_warning("string", "vector<double>", name, value_);
    eckit::Translator<std::string, double> translate;
    eckit::Tokenizer parse("/");

    std::vector<std::string> v;
    parse(value_, v);
    value.clear();
    value.reserve(v.size());
    for (const std::string& j : v) {
        value.push_back(translate(j));
    }
}


template <>
void TSettings<std::vector<long>>::get(const std::string&, std::vector<long>& value) const {
    value = value_;
}


template <>
void TSettings<std::vector<double>>::get(const std::string&, std::vector<double>& value) const {
    value = value_;
}


template <>
void TSettings<int>::get(const std::string& name, std::string& value) const {
    conversion_warning("int", "string", name, value_);
    value = std::to_string(value_);
}


template <>
void TSettings<long>::get(const std::string& name, std::string& value) const {
    conversion_warning("long", "string", name, value_);
    value = std::to_string(value_);
}


template <>
void TSettings<size_t>::get(const std::string& name, std::string& value) const {
    conversion_warning("size_t", "string", name, value_);
    value = std::to_string(value_);
}


template <>
void TSettings<float>::get(const std::string& name, std::string& value) const {
    conversion_warning("float", "string", name, value_);
    value = std::to_string(value_);
}


template <>
void TSettings<double>::get(const std::string& name, std::string& value) const {
    conversion_warning("double", "string", name, value_);
    value = std::to_string(value_);
}


template <>
void TSettings<std::vector<int>>::get(const std::string& name, std::string& value) const {
    conversion_warning("vector<int>", "string", name, value_);
    value.clear();

    auto sep = "";
    for (auto& entry : value_) {
        value += sep + std::to_string(entry);
        sep = "/";
    }
}


template <>
void TSettings<std::vector<long>>::get(const std::string& name, std::string& value) const {
    conversion_warning("vector<long>", "string", name, value_);
    value.clear();

    auto sep = "";
    for (auto& entry : value_) {
        value += sep + std::to_string(entry);
        sep = "/";
    }
}


template <>
void TSettings<std::vector<size_t>>::get(const std::string& name, std::string& value) const {
    conversion_warning("vector<size_t>", "string", name, value_);
    value.clear();

    auto sep = "";
    for (auto& entry : value_) {
        value += sep + std::to_string(entry);
        sep = "/";
    }
}


template <>
void TSettings<std::vector<float>>::get(const std::string& name, std::string& value) const {
    conversion_warning("vector<float>", "string", name, value_);
    value.clear();

    auto sep = "";
    for (auto& entry : value_) {
        value += sep + std::to_string(entry);
        sep = "/";
    }
}


template <>
void TSettings<std::vector<double>>::get(const std::string& name, std::string& value) const {
    conversion_warning("vector<double>", "string", name, value_);
    value.clear();

    auto sep = "";
    for (auto& entry : value_) {
        value += sep + std::to_string(entry);
        sep = "/";
    }
}


template <>
void TSettings<std::vector<std::string>>::get(const std::string& name, std::string& value) const {
    conversion_warning("vector<string>", "string", name, value_);
    value.clear();
    std::string sep;
    for (auto& entry : value_) {
        value += sep + entry;
        sep = "/";
    }
}


SimpleParametrisation::SimpleParametrisation() = default;


SimpleParametrisation::~SimpleParametrisation() {
    reset();
}


void SimpleParametrisation::copyValuesTo(SimpleParametrisation& other) const {
    for (const auto& j : settings_) {
        j.second->copyValueTo(j.first, other);
    }
}


void SimpleParametrisation::swap(SimpleParametrisation& other) {
    settings_.swap(other.settings_);
}


bool SimpleParametrisation::has(const std::string& name) const {
    return settings_.find(name) != settings_.end();
}


size_t SimpleParametrisation::size() const {
    return settings_.size();
}


template <class T>
bool SimpleParametrisation::_get(const std::string& name, T& value) const {
    auto j = settings_.find(name);
    if (j == settings_.end()) {
        return false;
    }
    j->second->get(name, value);
    // Log::debug() << "SimpleParametrisation::get(" << name << ") => " << value << std::endl;
    return true;
}


bool SimpleParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool SimpleParametrisation::get(const std::string&, std::vector<std::string>&) const {
    NOTIMP;
}


template <class T>
void SimpleParametrisation::_set(const std::string& name, const T& value) {
    auto j = settings_.find(name);
    if (j != settings_.end()) {
        delete j->second;
    }
    settings_[name] = new TSettings<T>(value);
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const char* value) {
    _set(name, std::string(value));
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, float value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::string& value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, bool value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, long value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, long long value) {
    _set(name, long(value));
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, size_t value) {
    // TODO: Support unsigned properly
    ASSERT(size_t(long(value)) == value);
    _set(name, long(value));
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<int>& value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, double value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, int value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::clear(const std::string& name) {
    auto j = settings_.find(name);
    if (j != settings_.end()) {
        delete j->second;
        settings_.erase(j);
    }
    return *this;
}


SimpleParametrisation& SimpleParametrisation::reset() {
    for (const auto& j : settings_) {
        delete j.second;
    }
    settings_.clear();
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<long>& value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<long long>& value) {
    std::vector<long> value_long(value.begin(), value.end());
    _set(name, value_long);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<size_t>& value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<float>& value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<double>& value) {
    _set(name, value);
    return *this;
}


SimpleParametrisation& SimpleParametrisation::set(const std::string& name, const std::vector<std::string>& value) {
    _set(name, value);
    return *this;
}


void SimpleParametrisation::print(std::ostream& out) const {
    if (eckit::format(out) == Log::applicationFormat) {
        const char* sep = "";
        for (const auto& j : settings_) {
            out << sep << "--" << j.first << "=" << *(j.second);
            sep = " ";
        }
        return;
    }

    eckit::JSON j(out);
    json(j);
}


void SimpleParametrisation::json(eckit::JSON& s) const {
    s.startObject();
    for (const auto& j : settings_) {
        s << j.first << *(j.second);
    }
    s.endObject();
}


bool SimpleParametrisation::empty() const {
    return size() == 0;
}


bool SimpleParametrisation::matches(const MIRParametrisation& other) const {
    for (const auto& j : settings_) {
        if (!j.second->match(j.first, other)) {
            Log::debug() << "SimpleParametrisation::matches: no (" << j.first << " different to " << *(j.second) << ")"
                         << std::endl;
            return false;
        }
    }
    Log::debug() << "SimpleParametrisation::matches: yes" << std::endl;
    return true;
}


}  // namespace param
}  // namespace mir

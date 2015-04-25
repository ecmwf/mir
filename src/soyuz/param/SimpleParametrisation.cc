/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "soyuz/param/SimpleParametrisation.h"

#include "eckit/exception/Exceptions.h"

#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"

 #include "eckit/types/Types.h"

template<class T>
inline std::ostream& operator<<(std::ostream& s,const std::vector<T>& v)
{
    return eckit::__print_list(s,v);
}


namespace mir {
namespace param {

class Setting {
  public:
    virtual ~Setting() {};
    virtual void get(const std::string& name, std::string& value) const = 0;
    virtual void get(const std::string& name, bool& value) const = 0;
    virtual void get(const std::string& name, long& value) const = 0;
    virtual void get(const std::string& name, double& value) const = 0;
    virtual void get(const std::string& name, std::vector<long>& value) const = 0;
    virtual void get(const std::string& name, std::vector<double>& value) const = 0;
};

template<class T>
class TSettings : public Setting {
    T value_;
  public:
    TSettings(const T& value): value_(value) {}
    virtual void get(const std::string& name, std::string& value) const;
    virtual void get(const std::string& name, bool& value) const;
    virtual void get(const std::string& name, long& value) const;
    virtual void get(const std::string& name, double& value) const;
    virtual void get(const std::string& name, std::vector<long>& value) const;
    virtual void get(const std::string& name, std::vector<double>& value) const;
};

//==========================================================

class CannotConvert : public eckit::Exception {
public:
    CannotConvert(const char* from, const char* to, const std::string& name) {
        eckit::StrStream os;
        os << "Cannot convert from " << from << " to " << to << " (requesting " << name << ")" << eckit::StrStream::ends;
        reason(std::string(os));
    }
};

// We will implement convertion as needed

template<> void TSettings<bool>::get(const std::string& name, std::string& value) const {
    throw CannotConvert("bool", "string", name);
}
template<> void TSettings<bool>::get(const std::string& name, bool& value) const {
    value = value_;
}
template<> void TSettings<bool>::get(const std::string& name, long& value) const {
    throw CannotConvert("bool", "long", name);
}
template<> void TSettings<bool>::get(const std::string& name, double& value) const {
    throw CannotConvert("bool", "value", name);
}
template<> void TSettings<bool>::get(const std::string& name, std::vector<long>& value) const {
    throw CannotConvert("bool", "vector<long>", name);
}
template<> void TSettings<bool>::get(const std::string& name, std::vector<double>& value) const {
    throw CannotConvert("bool", "vector<double>", name);
}
//==========================================================

template<> void TSettings<long>::get(const std::string& name, std::string& value) const {
    throw CannotConvert("long", "string", name);
}
template<> void TSettings<long>::get(const std::string& name, bool& value) const {
    throw CannotConvert("long", "bool", name);
}
template<> void TSettings<long>::get(const std::string& name, long& value) const {
    value = value_;
}
template<> void TSettings<long>::get(const std::string& name, double& value) const {
    throw CannotConvert("long", "double", name);
}
template<> void TSettings<long>::get(const std::string& name, std::vector<long>& value) const {
    throw CannotConvert("long", "vector<long>", name);
}
template<> void TSettings<long>::get(const std::string& name, std::vector<double>& value) const {
    throw CannotConvert("long", "vector<double>", name);
}
//==========================================================

template<> void TSettings<double>::get(const std::string& name, std::string& value) const {
    throw CannotConvert("double", "string", name);
}
template<> void TSettings<double>::get(const std::string& name, bool& value) const {
    throw CannotConvert("double", "bool", name);
}
template<> void TSettings<double>::get(const std::string& name, long& value) const {
    throw CannotConvert("double", "long", name);
}
template<> void TSettings<double>::get(const std::string& name, double& value) const {
    value = value_;
}
template<> void TSettings<double>::get(const std::string& name, std::vector<long>& value) const {
    throw CannotConvert("double", "vector<long>", name);
}
template<> void TSettings<double>::get(const std::string& name, std::vector<double>& value) const {
    throw CannotConvert("double", "vector<double>", name);
}
//==========================================================

template<> void TSettings<std::string>::get(const std::string& name, std::string& value) const {
    value = value_;
}
template<> void TSettings<std::string>::get(const std::string& name, bool& value) const {
    throw CannotConvert("string", "bool", name);
}
template<> void TSettings<std::string>::get(const std::string& name, long& value) const {
    throw CannotConvert("string", "value", name);
}
template<> void TSettings<std::string>::get(const std::string& name, double& value) const {
    throw CannotConvert("string", "double", name);
}
template<> void TSettings<std::string>::get(const std::string& name, std::vector<long>& value) const {
    throw CannotConvert("string", "vector<long>", name);
}
template<> void TSettings<std::string>::get(const std::string& name, std::vector<double>& value) const {
    throw CannotConvert("string", "vector<double>", name);
}
//==========================================================

template<> void TSettings<std::vector<long> >::get(const std::string& name, std::string& value) const {
    throw CannotConvert("vector<long>", "string", name);
}
template<> void TSettings<std::vector<long> >::get(const std::string& name, bool& value) const {
    throw CannotConvert("vector<long>", "bool", name);
}
template<> void TSettings<std::vector<long> >::get(const std::string& name, long& value) const {
    throw CannotConvert("vector<long>", "long", name);
}
template<> void TSettings<std::vector<long> >::get(const std::string& name, double& value) const {
    throw CannotConvert("vector<long>", "double", name);
}
template<> void TSettings<std::vector<long> >::get(const std::string& name, std::vector<long>& value) const {
    value = value_;
}
template<> void TSettings<std::vector<long> >::get(const std::string& name, std::vector<double>& value) const {
    throw CannotConvert("vector<long>", "vector<double>", name);
}

//==========================================================
template<> void TSettings<std::vector<double> >::get(const std::string& name, std::string& value) const {
    throw CannotConvert("vector<double>", "string", name);
}
template<> void TSettings<std::vector<double> >::get(const std::string& name, bool& value) const {
    throw CannotConvert("vector<double>", "yyy", name);
}
template<> void TSettings<std::vector<double> >::get(const std::string& name, long& value) const {
    throw CannotConvert("vector<double>", "yyy", name);
}
template<> void TSettings<std::vector<double> >::get(const std::string& name, double& value) const {
    throw CannotConvert("vector<double>", "yyy", name);
}
template<> void TSettings<std::vector<double> >::get(const std::string& name, std::vector<long>& value) const {
    throw CannotConvert("vector<double>", "vector<long>", name);
}
template<> void TSettings<std::vector<double> >::get(const std::string& name, std::vector<double>& value) const {
    value = value_;
}

//==========================================================
SimpleParametrisation::SimpleParametrisation() {
}


SimpleParametrisation::~SimpleParametrisation() {
    for (std::map<std::string, Setting*>::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {
        delete (*j).second;
    }
}

bool SimpleParametrisation::has(const std::string& name) const {
    return settings_.find(name) != settings_.end();
}

size_t SimpleParametrisation::size() const {
    return settings_.size();
}

template<class T>
bool SimpleParametrisation::_get(const std::string& name, T& value) const {
    std::map<std::string, Setting*>::const_iterator j = settings_.find(name);
    if (j == settings_.end()) {
        return false;
    }
    (*j).second->get(name, value);
    eckit::Log::info() << "SimpleParametrisation::get(" << name << ") => " << value << std::endl;
    return true;
}

bool SimpleParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}

template<class T>
void SimpleParametrisation::_set(const std::string& name, const T& value) {
    std::map<std::string, Setting*>::iterator j = settings_.find(name);
    if (j != settings_.end()) {
        delete (*j).second;
    }
    settings_[name] = new TSettings<T>(value);
}

void SimpleParametrisation::set(const std::string& name, const char* value) {
    _set(name, std::string(value));
}

void SimpleParametrisation::set(const std::string& name, const std::string& value) {
    _set(name, value);
}

void SimpleParametrisation::set(const std::string& name, bool value) {
    _set(name, value);
}

void SimpleParametrisation::set(const std::string& name, long value) {
    _set(name, value);
}

void SimpleParametrisation::set(const std::string& name, double value) {
    _set(name, value);
}

void SimpleParametrisation::set(const std::string& name, std::vector<long>& value) {
    _set(name, value);
}

void SimpleParametrisation::set(const std::string& name, std::vector<double>& value) {
    _set(name, value);
}


}  // namespace param
}  // namespace mir


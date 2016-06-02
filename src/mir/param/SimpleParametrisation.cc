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


#include "mir/param/SimpleParametrisation.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/parser/JSON.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/types/Types.h"
#include "eckit/utils/Translator.h"
#include "eckit/value/Value.h"

#include "mir/param/DelayedParametrisation.h"
#include "mir/log/MIR.h"

namespace mir {
namespace param {

class Setting {
  public:
    virtual ~Setting() {}
    virtual void get(const std::string &name, std::string &value) const = 0;
    virtual void get(const std::string &name, bool &value) const = 0;
    virtual void get(const std::string &name, long &value) const = 0;
    virtual void get(const std::string &name, double &value) const = 0;
    virtual void get(const std::string &name, std::vector<long> &value) const = 0;
    virtual void get(const std::string &name, std::vector<double> &value) const = 0;

    virtual bool match(const std::string &name, const MIRParametrisation &) const = 0;
    virtual void copyValueTo(const std::string &name, SimpleParametrisation &) const = 0;

    virtual void print(std::ostream &) const = 0;
    virtual void json(eckit::JSON&) const = 0;

    friend std::ostream &operator<<(std::ostream &s, const Setting &p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Setting& p) {
        p.json(s);
        return s;
    }
};

//================================================================================

class DelayedSetting : public Setting {
    DelayedParametrisation *delayed_;
  public:
    DelayedSetting(DelayedParametrisation *delayed): delayed_(delayed) {}

    virtual ~DelayedSetting() {
        delete delayed_;
    };

    virtual void get(const std::string &name, std::string &value) const {
        NOTIMP;
    }

    virtual void get(const std::string &name, bool &value) const {
        NOTIMP;
    }

    virtual void get(const std::string &name, long &value) const {
        delayed_->get(name, value);
    }

    virtual void get(const std::string &name, double &value) const {
        NOTIMP;
    }

    virtual void get(const std::string &name, std::vector<long> &value) const {
        NOTIMP;
    }

    virtual void get(const std::string &name, std::vector<double> &value) const {
        NOTIMP;
    }

    virtual bool match(const std::string &name, const MIRParametrisation &) const {
        NOTIMP;
    }

    virtual void print(std::ostream &out) const {
        out << "<DELAYED>";
    }

    virtual void json(eckit::JSON& out) const {
        out << "<DELAYED>";
    }

    void copyValueTo(const std::string &name, SimpleParametrisation &param) const  {
        NOTIMP;
    }

};

//================================================================================

template<class T>
class TSettings : public Setting {
    T value_;
  public:
    TSettings(const T &value): value_(value) {}

    virtual void get(const std::string &name, std::string &value) const;
    virtual void get(const std::string &name, bool &value) const;
    virtual void get(const std::string &name, long &value) const;
    virtual void get(const std::string &name, double &value) const;
    virtual void get(const std::string &name, std::vector<long> &value) const;
    virtual void get(const std::string &name, std::vector<double> &value) const;

    virtual bool match(const std::string &name, const MIRParametrisation &other) const {
        T value;
        if (other.get(name, value)) {
            return value_ == value;
        }
        return false;
    }

    void copyValueTo(const std::string &name, SimpleParametrisation &param) const  {
        param.set(name, value_);
    }

    virtual void print(std::ostream &out) const {
        out << value_;
    }

    virtual void json(eckit::JSON& out) const {
        out << value_;
    }
};

template<class T>
static void _put(std::ostream &out, const std::vector<T>& v) {
    const char* sep = "";
    const char* comma = ", ";
    if (eckit::format(out) == eckit::Log::applicationFormat) {
        comma = "/";
    }
    for(size_t i = 0; i < v.size(); i++) {
        out << sep << v[i];
        sep = comma;
    }
}

template<>
void TSettings<std::vector<long> >::print(std::ostream &out) const {
    _put(out, value_);
}

template<>
void TSettings<std::vector<double> >::print(std::ostream &out) const {
    _put(out, value_);
}

//==========================================================


//==========================================================

class CannotConvert : public eckit::Exception {
  public:
    template<class T>
    CannotConvert(const char *from, const char *to, const std::string &name, const T &value) {
        std::ostringstream os;
        os << "Cannot convert " << value << " from " << from << " to " << to << " (requesting " << name << ")";
        reason(os.str());
    }
};

template<class T>
static void convertion_warning(const char *from, const char *to, const std::string &name, const T &value) {
    // eckit::Log::warning() << "   +++ WARNING: Converting " << value << " from " << from << " to " << to << " (requesting " << name << ")" << std::endl;
}

// We will implement convertion as needed

template<> void TSettings<bool>::get(const std::string &name, std::string &value) const {
    throw CannotConvert("bool", "string", name, value_);
}
template<> void TSettings<bool>::get(const std::string &name, bool &value) const {
    value = value_;
}
template<> void TSettings<bool>::get(const std::string &name, long &value) const {
    throw CannotConvert("bool", "long", name, value_);
}
template<> void TSettings<bool>::get(const std::string &name, double &value) const {
    throw CannotConvert("bool", "double", name, value_);
}
template<> void TSettings<bool>::get(const std::string &name, std::vector<long> &value) const {
    throw CannotConvert("bool", "vector<long>", name, value_);
}
template<> void TSettings<bool>::get(const std::string &name, std::vector<double> &value) const {
    throw CannotConvert("bool", "vector<double>", name, value_);
}

//==========================================================

template<> void TSettings<long>::get(const std::string &name, std::string &value) const {
    throw CannotConvert("long", "string", name, value_);
}
template<> void TSettings<long>::get(const std::string &name, bool &value) const {
    throw CannotConvert("long", "bool", name, value_);
}
template<> void TSettings<long>::get(const std::string &name, long &value) const {
    value = value_;
}
template<> void TSettings<long>::get(const std::string &name, double &value) const {
    throw CannotConvert("long", "double", name, value_);
}
template<> void TSettings<long>::get(const std::string &name, std::vector<long> &value) const {
    throw CannotConvert("long", "vector<long>", name, value_);
}
template<> void TSettings<long>::get(const std::string &name, std::vector<double> &value) const {
    throw CannotConvert("long", "vector<double>", name, value_);
}
//==========================================================

template<> void TSettings<double>::get(const std::string &name, std::string &value) const {
    throw CannotConvert("double", "string", name, value_);
}
template<> void TSettings<double>::get(const std::string &name, bool &value) const {
    throw CannotConvert("double", "bool", name, value_);
}
template<> void TSettings<double>::get(const std::string &name, long &value) const {
    throw CannotConvert("double", "long", name, value_);
}
template<> void TSettings<double>::get(const std::string &name, double &value) const {
    value = value_;
}
template<> void TSettings<double>::get(const std::string &name, std::vector<long> &value) const {
    throw CannotConvert("double", "vector<long>", name, value_);
}
template<> void TSettings<double>::get(const std::string &name, std::vector<double> &value) const {
    throw CannotConvert("double", "vector<double>", name, value_);
}
//==========================================================

template<> void TSettings<std::string>::get(const std::string &name, std::string &value) const {
    value = value_;
}
template<> void TSettings<std::string>::get(const std::string &name, bool &value) const {
    convertion_warning("string", "bool", name, value_);
    eckit::Translator<std::string, long> translate;
    value = translate(value_) != 0;
}

template<> void TSettings<std::string>::get(const std::string &name, long &value) const {
    convertion_warning("string", "long", name, value_);
    eckit::Translator<std::string, long> translate;
    value = translate(value_);
}
template<> void TSettings<std::string>::get(const std::string &name, double &value) const {
    convertion_warning("string", "double", name, value_);
    eckit::Translator<std::string, double> translate;
    value = translate(value_);
}
template<> void TSettings<std::string>::get(const std::string &name, std::vector<long> &value) const {
    throw CannotConvert("string", "vector<long>", name, value_);
}
template<> void TSettings<std::string>::get(const std::string &name, std::vector<double> &value) const {
    convertion_warning("string", "vector<double>", name, value_);
    eckit::Translator<std::string, double> translate;
    eckit::Tokenizer parse("/");

    std::vector<std::string> v;
    parse(value_, v);
    value.clear();
    value.reserve(v.size());
    for (std::vector<std::string>::const_iterator j = v.begin(); j != v.end(); ++j) {
        value.push_back(translate(*j));
    }
}
//==========================================================

template<> void TSettings<std::vector<long> >::get(const std::string &name, std::string &value) const {
    throw CannotConvert("vector<long>", "string", name, value_);
}
template<> void TSettings<std::vector<long> >::get(const std::string &name, bool &value) const {
    throw CannotConvert("vector<long>", "bool", name, value_);
}
template<> void TSettings<std::vector<long> >::get(const std::string &name, long &value) const {
    throw CannotConvert("vector<long>", "long", name, value_);
}
template<> void TSettings<std::vector<long> >::get(const std::string &name, double &value) const {
    throw CannotConvert("vector<long>", "double", name, value_);
}
template<> void TSettings<std::vector<long> >::get(const std::string &name, std::vector<long> &value) const {
    value = value_;
}
template<> void TSettings<std::vector<long> >::get(const std::string &name, std::vector<double> &value) const {
    throw CannotConvert("vector<long>", "vector<double>", name, value_);
}

//==========================================================
template<> void TSettings<std::vector<double> >::get(const std::string &name, std::string &value) const {
    throw CannotConvert("vector<double>", "string", name, value_);
}
template<> void TSettings<std::vector<double> >::get(const std::string &name, bool &value) const {
    throw CannotConvert("vector<double>", "bool", name, value_);
}
template<> void TSettings<std::vector<double> >::get(const std::string &name, long &value) const {
    throw CannotConvert("vector<double>", "long", name, value_);
}
template<> void TSettings<std::vector<double> >::get(const std::string &name, double &value) const {
    throw CannotConvert("vector<double>", "double", name, value_);
}
template<> void TSettings<std::vector<double> >::get(const std::string &name, std::vector<long> &value) const {
    throw CannotConvert("vector<double>", "vector<long>", name, value_);
}
template<> void TSettings<std::vector<double> >::get(const std::string &name, std::vector<double> &value) const {
    value = value_;
}


//==========================================================
SimpleParametrisation::SimpleParametrisation() {
}


SimpleParametrisation::~SimpleParametrisation() {
    for (SettingsMap::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {
        delete (*j).second;
    }
}

bool SimpleParametrisation::has(const std::string &name) const {
    return settings_.find(name) != settings_.end();
}

size_t SimpleParametrisation::size() const {
    return settings_.size();
}

template<class T>
bool SimpleParametrisation::_get(const std::string &name, T &value) const {
    SettingsMap::const_iterator j = settings_.find(name);
    if (j == settings_.end()) {
        return false;
    }
    (*j).second->get(name, value);
    eckit::Log::trace<MIR>() << "SimpleParametrisation::get(" << name << ") => " << value << std::endl;
    return true;
}

bool SimpleParametrisation::get(const std::string &name, std::string &value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string &name, bool &value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string &name, long &value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string &name, double &value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string &name, std::vector<long> &value) const {
    return _get(name, value);
}

bool SimpleParametrisation::get(const std::string &name, std::vector<double> &value) const {
    return _get(name, value);
}

template<class T>
void SimpleParametrisation::_set(const std::string &name, const T &value) {
    SettingsMap::iterator j = settings_.find(name);
    if (j != settings_.end()) {
        delete (*j).second;
    }
    settings_[name] = new TSettings<T>(value);
}

// FIXME: can we do this in a more elegant way?
template<>
void SimpleParametrisation::_set(const std::string &name, const eckit::Value& value) {
    if (value.isBool()) {
        _set<bool>(name, value);
    } else if (value.isDouble()) {
        _set<double>(name, value);
    } else if (value.isNumber()) {
        _set<long>(name, value);
    } else if (value.isString()) {
        _set<std::string>(name, value);
    } else if (value.isList()) {
        eckit::ValueList v = value;
        if (v[0].isDouble()) {
            std::vector<double> d;
            for (eckit::ValueList::const_iterator it = v.begin(); it != v.end(); ++it)
                d.push_back(double(*it));
            _set(name, d);
        } else if (v[0].isNumber()) {
            std::vector<long> l;
            for (eckit::ValueList::const_iterator it = v.begin(); it != v.end(); ++it)
                l.push_back(long(*it));
            _set(name, l);
        } else {
            throw eckit::BadParameter("Vector contains invalid type", Here());
        }
    } else {
        throw eckit::BadParameter("Map contains invalid type", Here());
    }
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, const char *value) {
    _set(name, std::string(value));
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, const std::string &value) {
    _set(name, value);
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, bool value) {
    _set(name, value);
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, long value) {
    _set(name, value);
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, size_t value) {
    // TODO: Support unsigned properly
    ASSERT(long(value) == value);
    _set(name, long(value));
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, double value) {
    _set(name, value);
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, DelayedParametrisation *value) {
    SettingsMap::iterator j = settings_.find(name);
    if (j != settings_.end()) {
        delete (*j).second;
    }
    settings_[name] = new DelayedSetting(value);
    return *this;
}

SimpleParametrisation& SimpleParametrisation::clear(const std::string &name) {
    SettingsMap::iterator j = settings_.find(name);
    if (j != settings_.end()) {
        delete (*j).second;
        settings_.erase(j);
    }
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, const std::vector<long> &value) {
    _set(name, value);
    return *this;
}

SimpleParametrisation& SimpleParametrisation::set(const std::string &name, const std::vector<double> &value) {
    _set(name, value);
    return *this;
}

void SimpleParametrisation::print(std::ostream &out) const {
    const char *sep = "";
    const char *comma = ",";
    const char *extra = "";

    if (eckit::format(out) == eckit::Log::applicationFormat) {
        extra ="--";
        comma = " ";
    }

    for (SettingsMap::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {
        out << sep;
        out << extra << (*j).first << "=" << *((*j).second);
        sep = comma;
    }
}

void SimpleParametrisation::json(eckit::JSON& s) const {
    s.startObject();
    for (SettingsMap::const_iterator j = settings_.begin(); j != settings_.end(); ++j)
        s << (*j).first << *((*j).second);
    s.endObject();
}

bool SimpleParametrisation::matches(const MIRParametrisation &other) const {
    eckit::Log::trace<MIR>() << "SimpleParametrisation::matches " << other << std::endl;
    for (SettingsMap::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {

        if ((*j).second->match((*j).first, other)) {
            eckit::Log::trace<MIR>() << "Matching parametrisation: " << (*j).first << "="
                               << *((*j).second) << std::endl;
            return true;
        } else {
            eckit::Log::trace<MIR>() << "Not matching parametrisation: " << (*j).first << "="
                               << *((*j).second) << std::endl;
        }

    }
    return false;
}

void SimpleParametrisation::copyValuesTo(SimpleParametrisation& other) const {
    for (SettingsMap::const_iterator j = settings_.begin(); j != settings_.end(); ++j) {
        (*j).second->copyValueTo((*j).first, other);
    }
}

}  // namespace param
}  // namespace mir


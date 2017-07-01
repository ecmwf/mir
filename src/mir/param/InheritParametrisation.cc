/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Nov 2016


#include "mir/param/InheritParametrisation.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/StringTools.h"
#include "eckit/utils/Translator.h"


namespace {


bool string_contains_paramIds(const std::string& str, std::vector<long>& ids) {
    ids.clear();
    if (str.find_first_not_of("0123456789" "/") != std::string::npos) {
        return false;
    }

    std::vector<std::string> v = eckit::StringTools::split("/", str);
    ids.reserve(v.size());

    for (const std::string& i: v) {
        long id = 0;
        if (i.length() && (id = eckit::Translator<std::string, long>()(i))) {
            ids.push_back(id);
        }
    }

    return ids.size();
}


bool string_contains_label(const std::string& str) {
    const char* alnum =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789"
            "_=-.";
    return (str.find_first_not_of(alnum) == std::string::npos);
}


}  // (anonymous namespace)


namespace mir {
namespace param {


InheritParametrisation::InheritParametrisation() : parent_(0) {}


InheritParametrisation::~InheritParametrisation() {
    for(auto j = children_.begin(); j != children_.end(); ++j) {
        delete (*j);
    }
}


void InheritParametrisation::fill(const eckit::ValueMap& map) {
    for (eckit::ValueMap::const_iterator i = map.begin(); i != map.end(); ++i) {
        if (i->second.isMap()) {
            std::vector< long> ids;

            if (string_contains_paramIds(i->first, ids)) {
                addChild(new InheritParametrisation(this, ids)).fill(i->second);
            } else if (string_contains_label(i->first)) {
                addChild(new InheritParametrisation(this, i->first)).fill(i->second);
            }

        } else if (!has(i->first)) {

            set(i->first, std::string(i->second));

        }
    }
}


void InheritParametrisation::fill(const InheritParametrisation& filler) {
    const std::string label = filler.label_;
    ASSERT(string_contains_label(label));

    // recursively inherit all fill parametrisation traits
    std::string fill_label;
    while (get(label, fill_label)) {
        clearFromChildren(label);
        filler.pick(fill_label).inherit(*this);
    }

    // descendants do the same
    for (InheritParametrisation* me: children_) {
        me->fill(filler);
    }
}


const InheritParametrisation& InheritParametrisation::pick(const MIRParametrisation& metadata) const {
    long id = 0;
    metadata.get("paramId", id);
    return pick(id, metadata);
}


const InheritParametrisation& InheritParametrisation::pick(const long& paramId, const MIRParametrisation& metadata) const {
    if (paramId > 0 ) {
        for (InheritParametrisation* me: children_) {
            ASSERT(me != this);
            if (me->matchesId(paramId) && me->matchesMetadata(metadata)) {
                return me->pick(paramId, metadata);
            }
        }
    }
    return *this;
}


const InheritParametrisation& InheritParametrisation::pick(const std::string& str) const {
    return pick(eckit::StringTools::split("/", str));
}


const InheritParametrisation& InheritParametrisation::pick(const std::vector< std::string >& labels) const {
    if (labels.size()) {
        for (const InheritParametrisation* me: children_) {
            const std::string& label = labels[0];
            if (!string_contains_label(label)) {
                throw eckit::UserError("InheritParametrisation: invalid label '" + label + "' (from '" + eckit::StringTools::join("/", labels) + "')");
            }
            if (me->matchesLabel(label)) {
                return me->pick(std::vector<std::string>(labels.begin()+1, labels.end()));
            }
        }
        throw eckit::UserError("InheritParametrisation: cannot locate label '" + eckit::StringTools::join("/", labels) + "' under '" + labelHierarchy() + "'");
    }
    return *this;
}

bool InheritParametrisation::empty() const {
    return children_.size()==0 && SimpleParametrisation::empty();
}


InheritParametrisation& InheritParametrisation::clear(const std::string& name) {
    SimpleParametrisation::clear(name);
    if (parent_ != NULL) {
        const_cast< InheritParametrisation* >(parent_)->clear(name);
    }
    return *this;
}


bool InheritParametrisation::has(const std::string& name) const {
    return SimpleParametrisation::has(name) || (parent_ != NULL && parent_->has(name));
}


bool InheritParametrisation::get(const std::string& name, std::string& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, bool& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, int& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, long& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, float& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, double& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, std::vector<int>& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, std::vector<long>& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, std::vector<float>& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, std::vector<double>& value) const {
    return _get(name, value);
}


bool InheritParametrisation::get(const std::string& name, std::vector<std::string>& value) const {
    return _get(name, value);
}


void InheritParametrisation::print(std::ostream& out) const {
    out << "InheritParametrisation["
        <<  "label=" << label_
        << ",empty?" << empty();

    out << ",paramIds=[";
    const char* sep = "";
    size_t count = 0;
    for (long id: paramIds_) {
        if (++count > 10) {
            out << ",...";
            break;
        }
        out << sep << id;
        sep = ",";
    }
    out << "]";

    out << ",SimpleParametrisation[";
    SimpleParametrisation::print(out);
    out << "]";

    if (!children_.empty()) {
        out << ",children[";
        sep = "\n\t";
        count = 0;
        for (const InheritParametrisation* me: children_) {
            if (++count > 10) {
                out << ",...";
                break;
            }
            out << sep << *me;
            sep = ",\n\t";
        }
        out << "]";
    }

    out << "]";
}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::string& label) :
    parent_(parent), label_(label) {
    ASSERT(parent_);
    ASSERT(string_contains_label(label_));
}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& ids) :
    parent_(parent), paramIds_(ids) {
    ASSERT(parent_);
    ASSERT(std::find(paramIds_.begin(), paramIds_.end(), 0) == paramIds_.end());
}


template<typename T>
bool InheritParametrisation::_get(const std::string& name, T& value) const {
    return SimpleParametrisation::get(name, value) || (parent_ != NULL && parent_->_get(name, value));
}


bool InheritParametrisation::matchesId(long id) const {

    const InheritParametrisation* who = this;
    size_t check = 0;
    while (!(who->paramIds_.size()) && (who->parent_ != NULL)) {
        ASSERT(check++ < 50);
        who = who->parent_;
    }

    const std::vector<long>& ids = who->paramIds_;
    return (std::find(ids.begin(), ids.end(), id) != ids.end());
}


bool InheritParametrisation::matchesMetadata(const MIRParametrisation& metadata) const {
    if (label_.empty()) {
        return true;
    }

    std::vector<std::string> key_value = eckit::StringTools::split("=", label_);
    key_value.resize(2);
    const std::string& key = key_value[0];
    const std::string& val = key_value[1];

    std::string meta_value;
    return key.length()
            && metadata.get(key, meta_value)
            && (val == meta_value);
}


bool InheritParametrisation::matchesLabel(const std::string& label) const {
    ASSERT(label.length());
    return label_.length() && (label_ == label);
}


InheritParametrisation& InheritParametrisation::addChild(InheritParametrisation* who) {
    ASSERT(who);
    ASSERT(who != this);
    children_.push_back(who);
    return *who;
}


InheritParametrisation& InheritParametrisation::clearFromChildren(const std::string& name) {
    for (InheritParametrisation* me: children_) {
        me->clearFromChildren(name);
    }
    SimpleParametrisation::clear(name);
    return *this;
}


void InheritParametrisation::inherit(SimpleParametrisation& parametrisation) const {
    copyValuesTo(parametrisation, false);
    if (parent_ != NULL) {
        parent_->inherit(parametrisation);
    }
}


std::string InheritParametrisation::labelHierarchy() const {
    std::string here;
    for (const InheritParametrisation* who = this; who != NULL; who = who->parent_) {
        here.insert(0, "/" + who->label_);
    }
    return here;
}


}  // namespace param
}  // namespace mir

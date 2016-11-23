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


#include "mir/config/InheritParametrisation.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include "eckit/exception/Exceptions.h"
#include "eckit/parser/StringTools.h"
#include "eckit/utils/Translator.h"
#include "mir/param/MIRParametrisation.h"


namespace {


bool string_contains_paramIds(const std::string& str, std::vector<long>& ids) {
    ids.clear();
    if (str.find_first_not_of("0123456789/") != std::string::npos) {
        return false;
    }

    std::vector<std::string> v = eckit::StringTools::split("/", str);
    ids.reserve(v.size());

    for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
        long id = 0;
        if (i->length() && (id = eckit::Translator<std::string, long>()(*i))) {
            ids.push_back(id);
        }
    }

    return ids.size();
}


bool string_contains_keys(const std::string& str, std::vector<std::string>& keys) {

    keys = eckit::StringTools::split("=", str);
    for (std::vector<std::string>::iterator sit = keys.begin(); sit != keys.end(); ++sit) {
        *sit = eckit::StringTools::trim(*sit);
    }

    return keys.size();
}


}  // (anonymous namespace)


namespace mir {
namespace config {


InheritParametrisation::InheritParametrisation() : parent_(NULL) {}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::string& key, const std::string& value) :
    parent_(parent), key_(key), value_(value) {
    ASSERT(parent_);
    ASSERT(key_.length());
}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& paramIds) :
    parent_(parent), paramIds_(paramIds) {
    ASSERT(parent_);
    ASSERT(std::find(paramIds_.begin(), paramIds_.end(), 0) == paramIds_.end());
}


InheritParametrisation::~InheritParametrisation() {
}


InheritParametrisation& InheritParametrisation::child(InheritParametrisation* who) {
    ASSERT(who);
    children_.push_back(who);
    return *who;
}


void InheritParametrisation::fill(const eckit::ValueMap& map) {
    for (eckit::ValueMap::const_iterator i = map.begin(); i != map.end(); ++i) {
        if (i->second.isMap()) {

            std::vector<long> ids;
            std::vector<std::string> keys;

            if (string_contains_paramIds(i->first, ids)) {
                child(new InheritParametrisation(this, ids)).fill(i->second);
            } else if (string_contains_keys(i->first, keys)) {
                if (keys.size() < 2) {
                    keys.resize(2);
                }
                child(new InheritParametrisation(this, keys[0], keys[1])).fill(i->second);
            }

        } else if (!has(i->first)) {

            set(i->first, std::string(i->second));

        }
    }
}


const InheritParametrisation& InheritParametrisation::pick(const long& paramId, const param::MIRParametrisation& metadata) const {
    for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); paramId > 0 && me!= children_.end(); ++me) {
        ASSERT(*me != this);
        if ((*me)->matches(paramId, metadata)) {
            return (*me)->pick(paramId, metadata);
        }
    }
    return *this;
}


const InheritParametrisation& InheritParametrisation::pick(const std::string& key, const std::string& value) const {
    for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        ASSERT(*me != this);
        if ((*me)->matches(key, value)) {
            return (*me)->pick(key, value);
        }
    }
    return *this;
}


void InheritParametrisation::inherit(param::SimpleParametrisation& who) const {
    copyValuesTo(who, false);
    if (parent_ != NULL) {
        parent_->inherit(who);
    }
}


bool InheritParametrisation::matches(const long& paramId, const param::MIRParametrisation& metadata) const {
    const std::vector<long>& ids = paramIds();
    if (std::find(ids.begin(), ids.end(), paramId) != ids.end()) {
        std::string value;
        return metadata.get(key_, value) && (value_ == value);
    }
    return false;
}


bool InheritParametrisation::matches(const std::string& key, const std::string& value) const {
    if (key_.length() && value_.length()) {
        return (key == key_) && (value == value_);
    }
    return false;
}


const std::vector<long>& InheritParametrisation::paramIds() const {
    const InheritParametrisation* who = this;
    size_t check = 0;
    while (!(who->paramIds_.size()) && (who->parent_ != NULL)) {
        ASSERT(check++ < 50);
        who = who->parent_;
    }
    return who->paramIds_;
}


bool InheritParametrisation::empty() const {
    return children_.size()==0 && SimpleParametrisation::empty();
}


void InheritParametrisation::print(std::ostream& out) const {
    out << "InheritParametrisation["
           "parent?"   << (parent_!=NULL)
        << ",empty?"    << empty()
        << ",paramIds=[";
    std::copy(paramIds_.begin(), paramIds_.end(), std::ostream_iterator<long>(out, ","));
    out << "]"
           ",metadata[" << key_ << "=" << value_ << "]"
        << ",SimpleParametrisation[";
    SimpleParametrisation::print(out);
    out << "]"
           ",children[";
    const char* sep = "";
    for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        out << sep << *(*me);
        sep = ",";
    }
    out << "]]";
}


}  // namespace config
}  // namespace mir

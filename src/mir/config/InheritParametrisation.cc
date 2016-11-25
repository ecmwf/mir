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


bool string_contains_labels(const std::string& str, std::vector<std::string>& labels) {
    labels.clear();
    const char* alnum =
            "ABCDEFGHIJKLMNOPQRSTUWXYZ"
            "abcdefghijklmnopqrstuwxyz"
            "0123456789_/=-";
    if (str.find_first_not_of(alnum) != std::string::npos) {
        return false;
    }

    std::vector<std::string> v = eckit::StringTools::split("/", str);
    labels.reserve(v.size());

    for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
        if (i->length()) {
            labels.push_back(*i);
        }
    }

    return labels.size();
}


}  // (anonymous namespace)


namespace mir {
namespace config {


InheritParametrisation::InheritParametrisation() : parent_(NULL) {}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::vector<std::string>& labels) :
    parent_(parent), labels_(labels) {
    ASSERT(parent_);
    ASSERT(labels_.size());
}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& ids) :
    parent_(parent), paramIds_(ids) {
    ASSERT(parent_);
    ASSERT(std::find(paramIds_.begin(), paramIds_.end(), 0) == paramIds_.end());
}


InheritParametrisation::~InheritParametrisation() {
    while (children_.size()) {
        delete children_.back();
        children_.pop_back();
    }
}


InheritParametrisation& InheritParametrisation::child(InheritParametrisation* who) {
    ASSERT(who);
    ASSERT(who != this);
    children_.push_back(who);
    return *who;
}


void InheritParametrisation::fill(const eckit::ValueMap& map) {
    for (eckit::ValueMap::const_iterator i = map.begin(); i != map.end(); ++i) {
        if (i->second.isMap()) {
            std::vector< long> ids;
            std::vector< std::string > labels;

            if (string_contains_paramIds(i->first, ids)) {
                child(new InheritParametrisation(this, ids)).fill(i->second);
            } else if (string_contains_labels(i->first, labels)) {
                child(new InheritParametrisation(this, labels)).fill(i->second);
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


const InheritParametrisation& InheritParametrisation::pick(const std::string& label) const {
    std::vector< std::string > labels;
    if (string_contains_labels(label, labels)) {
        for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
            ASSERT(*me != this);
            for (std::vector< std::string >::const_iterator l=labels.begin(); l!= labels.end(); ++l) {
                if ((*me)->matches(*l)) {
                    return (*me)->pick(label);
                }
            }
        }
    }
    return *this;
}


void InheritParametrisation::inherit(param::SimpleParametrisation& param) const {
    copyValuesTo(param, false);
    if (parent_ != NULL) {
        parent_->inherit(param);
    }
}


bool InheritParametrisation::matches(const long& paramId, const param::MIRParametrisation& metadata) const {

    // check if a parent node (or this one) has a list of parameters to check with
    const InheritParametrisation* who = this;
    size_t check = 0;
    while (!(who->paramIds_.size()) && (who->parent_ != NULL)) {
        ASSERT(check++ < 50);
        who = who->parent_;
    }

    const std::vector<long>& ids = who->paramIds_;
    if (std::find(ids.begin(), ids.end(), paramId) == ids.end()) {
        return false;
    }

    // check if label is according to given metadata
    if (labels_.empty()) {
        return true;
    }

    for (std::vector<std::string>::const_iterator l=labels_.begin(); l != labels_.end(); ++l) {

        std::vector<std::string> key_value = eckit::StringTools::split("=", *l);
        key_value.resize(2);
        const std::string& key = key_value[0];
        const std::string& val = key_value[1];

        std::string meta_value;
        if (key.length() && metadata.get(key, meta_value)) {
            return (val == meta_value);
        }
    }
    return false;
}


bool InheritParametrisation::matches(const std::string& label) const {
    ASSERT(label.length());
    if (std::find(labels_.begin(), labels_.end(), label) == labels_.end()) {
        return false;
    }
    return true;
}


bool InheritParametrisation::empty() const {
    return children_.size()==0 && SimpleParametrisation::empty();
}


void InheritParametrisation::print(std::ostream& out) const {
    out << "InheritParametrisation["
        <<  "empty?" << empty()
        << ",paramIds=[";
    std::copy(paramIds_.begin(), paramIds_.end(), std::ostream_iterator<long>(out, ","));
    out << "]"
           ",labels[";
    std::copy(labels_.begin(), labels_.end(), std::ostream_iterator<std::string>(out, ","));
    out << "]"
           ",SimpleParametrisation[";
    SimpleParametrisation::print(out);
    out << "]"
           ",children[";
    for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        out << "\t" << *(*me) << ",\n";
    }
    out << "]]";
}


}  // namespace config
}  // namespace mir

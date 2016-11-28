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

#include <algorithm>
#include <iostream>
#include <iterator>
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

    for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
        long id = 0;
        if (i->length() && (id = eckit::Translator<std::string, long>()(*i))) {
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
            "_=-";
    return (str.find_first_not_of(alnum) == std::string::npos);
}


}  // (anonymous namespace)


namespace mir {
namespace param {


InheritParametrisation::InheritParametrisation() : parent_(NULL) {}


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

            if (string_contains_paramIds(i->first, ids)) {
                child(new InheritParametrisation(this, ids)).fill(i->second);
            } else if (string_contains_label(i->first)) {
                child(new InheritParametrisation(this, i->first)).fill(i->second);
            }

        } else if (!has(i->first)) {

            set(i->first, std::string(i->second));

        }
    }
}


void InheritParametrisation::fill(const InheritParametrisation& filler) {
    const std::string fill_root_hierarchy = filler.labelHierarchy();
    const std::string fill_root_label = filler.label_;
    ASSERT(string_contains_label(fill_root_label));

    // recursively inherit all fill parametrisation traits
    std::string fill_label;
    while (get(fill_root_label, fill_label)) {
        clear(fill_root_label);
        filler.pick(fill_label).inherit(*this);
    }

    // descendants do the same
    for (std::vector< InheritParametrisation* >::iterator me=children_.begin(); me!= children_.end(); ++me) {
        (*me)->fill(filler);
    }
}


const InheritParametrisation& InheritParametrisation::pick(const long& paramId, const param::MIRParametrisation& metadata) const {
    for (std::vector< InheritParametrisation* >::const_iterator me = children_.begin(); paramId > 0 && me!= children_.end(); ++me) {
        ASSERT(*me != this);
        if ((*me)->matches(paramId, metadata)) {
            return (*me)->pick(paramId, metadata);
        }
    }
    return *this;
}


const InheritParametrisation& InheritParametrisation::pick(const std::string& str) const {
    return pick(eckit::StringTools::split("/", str));
}


const InheritParametrisation& InheritParametrisation::pick(const std::vector< std::string >& labels) const {
    for (std::vector< InheritParametrisation* >::const_iterator me = children_.begin(); labels.size() && me!= children_.end(); ++me) {
        const std::string& label = labels[0];
        if (!string_contains_label(label)) {
            std::ostringstream msg;
            msg << "MIRConfiguration: invalid label '" << label << "' (from '" << eckit::StringTools::join("/", labels) << "')";
            throw eckit::UserError(msg.str());
        }
        if ((*me)->matches(label)) {
            return (*me)->pick(std::vector<std::string>(labels.begin()+1, labels.end()));
        }
    }
    if (labels.size()) {
        std::ostringstream msg;
        msg << "MIRConfiguration: cannot locate label '" << eckit::StringTools::join("/", labels) << "' under '" << labelHierarchy() << "'";
        throw eckit::UserError(msg.str());
    }
    return *this;
}


void InheritParametrisation::inherit(param::SimpleParametrisation& param) const {
    copyValuesTo(param, false);
    if (parent_ != NULL) {
        parent_->inherit(param);
    }
}


std::string InheritParametrisation::labelHierarchy() const {
    std::string here;
    for (const InheritParametrisation* who = this; who != NULL; who = who->parent_) {
        here.insert(0, "/" + who->label_);
    }
    return here;
}


bool InheritParametrisation::matches(const long& paramId, const MIRParametrisation& metadata) const {

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
    if (!label_.empty()) {
        std::vector<std::string> key_value = eckit::StringTools::split("=", label_);
        key_value.resize(2);
        const std::string& key = key_value[0];
        const std::string& val = key_value[1];

        std::string meta_value;
        return key.length()
                && metadata.get(key, meta_value)
                && (val == meta_value);
    }
    return true;
}


bool InheritParametrisation::matches(const std::string& label) const {
    ASSERT(label.length());
    return label_.length() && (label_ == label);
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
           ",label=" << label_
        << ",SimpleParametrisation[";
    SimpleParametrisation::print(out);
    out << "]"
           ",children[";
    for (std::vector< InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        out << "\n\t" << *(*me) << ",";
    }
    out << "]]\n";
}


}  // namespace param
}  // namespace mir

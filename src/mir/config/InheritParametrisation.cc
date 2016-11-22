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


#include <algorithm>
#include <iostream>
#include <iterator>
#include "eckit/exception/Exceptions.h"
#include "mir/config/InheritParametrisation.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace config {


InheritParametrisation::InheritParametrisation() : parent_(NULL) {}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::string& key, const std::string& value) :
    parent_(parent), key_(key), value_(value) {
    ASSERT(parent_);
}


InheritParametrisation::InheritParametrisation(const InheritParametrisation* parent, const std::vector<long>& paramIds) :
    parent_(parent), paramIds_(paramIds) {
    ASSERT(parent_);
    ASSERT(std::find(paramIds_.begin(), paramIds_.end(), 0) == paramIds_.end());
}


InheritParametrisation::~InheritParametrisation() {
}


void InheritParametrisation::child(const InheritParametrisation* who) {
    children_.push_back(who);
}


bool InheritParametrisation::pick(const InheritParametrisation* who, const long& paramId, const param::MIRParametrisation& metadata) const {
    who = this;
    size_t check = 0;
    for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        ASSERT(check++ < 50);
        ASSERT(*me != this);
        ASSERT(*me);
        if ((*me)->matches(paramId, metadata)) {
            return pick(*me, paramId, metadata);
        }
    }
    return false;
}


bool InheritParametrisation::pick(const InheritParametrisation* who, const std::string& key, const std::string& value) const {
    who = this;
    size_t check = 0;
    for (std::vector< const InheritParametrisation* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        ASSERT(check++ < 50);
        ASSERT(*me != this);
        ASSERT(*me);
        if ((*me)->matches(key, value)) {
            return pick(*me, key, value);
        }
    }
    return false;
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

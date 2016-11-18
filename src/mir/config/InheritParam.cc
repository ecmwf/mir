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
#include "mir/config/InheritParam.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace config {


InheritParam::InheritParam() : parent_(NULL) {}


InheritParam::InheritParam(const InheritParam* parent, const std::string& key, const std::string& value) :
    parent_(parent), key_(key), value_(value) {
    ASSERT(parent_);
}


InheritParam::InheritParam(const InheritParam* parent, const std::vector<long>& paramIds) :
    parent_(parent), paramIds_(paramIds) {
    ASSERT(parent_);
    ASSERT(std::find(paramIds_.begin(), paramIds_.end(), 0) == paramIds_.end());
}


InheritParam::~InheritParam() {

}


void InheritParam::child(const InheritParam* who) {
    children_.push_back(who);
}


bool InheritParam::pick(const InheritParam* who, const long& paramId, const param::MIRParametrisation& metadata) const {
    who = this;
    size_t check = 0;
    for (std::vector< const InheritParam* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        ASSERT(check++ < 50);
        ASSERT(*me != this);
        ASSERT(*me);
        if ((*me)->matches(paramId, metadata)) {
            return pick(*me, paramId, metadata);
        }
    }
    return false;
}


void InheritParam::inherit(param::SimpleParametrisation& who) const {
    copyValuesTo(who, false);
    if (parent_ != NULL) {
        parent_->inherit(who);
    }
}


bool InheritParam::matches(const long& paramId, const param::MIRParametrisation& metadata) const {
    const std::vector<long>&  ids = paramIds();
    if (std::find(ids.begin(), ids.end(), paramId) != ids.end()) {
        std::string value;
        return metadata.get(key_, value) && (value_ == value);
    }
    return false;
}


const std::vector<long>& InheritParam::paramIds() const {
    const InheritParam* who = this;
    size_t check = 0;
    while (!(who->paramIds_.size()) && (who->parent_ != NULL)) {
        ASSERT(check++ < 50);
        who = who->parent_;
    }
    return who->paramIds_;
}


bool InheritParam::empty() const {
    return children_.size()==0 && SimpleParametrisation::empty();
}


void InheritParam::print(std::ostream& out) const {
    out << "InheritParam["
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
    for (std::vector< const InheritParam* >::const_iterator me=children_.begin(); me!= children_.end(); ++me) {
        out << sep << *(*me);
        sep = ",";
    }
    out << "]]";
}


}  // namespace config
}  // namespace mir

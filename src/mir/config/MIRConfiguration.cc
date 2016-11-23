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


#include "mir/config/MIRConfiguration.h"

#include <iostream>
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/JSONParser.h"
#include "eckit/parser/StringTools.h"
#include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace config {


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


const MIRConfiguration& MIRConfiguration::instance() {
    static MIRConfiguration instance_;
    return instance_;
}


MIRConfiguration::MIRConfiguration() {

    fillKey_ = "class";
    configFile_ = "configuration.json";
    configDir_  = "~mir/etc/mir";


    // open and parse configuration file
    eckit::PathName path(configFile_);
    if (!path.exists()){
        path = eckit::PathName(configDir_) / path;
    }

    eckit::Log::debug<LibMir>() << "Loading configuration from '" << path << "'" << std::endl;
    std::ifstream in(path.asString().c_str());
    if (!in) {
        throw eckit::CantOpenFile(path);
    }

    eckit::JSONParser parser(in);
    const eckit::ValueMap j = parser.parse();


    // create paramId/metadata hierarchy
    root_.reset(new InheritParametrisation());
    parseInheritMap(root_.get(), j);
    eckit::Log::debug<LibMir>() << "MIRConfiguration root: " << *root_ << std::endl;


    // create filling ("class") hierarchy
    fill_.reset(new InheritParametrisation());
    eckit::ValueMap::const_iterator fill_it = j.find(fillKey_);
    if (fill_it != j.end()) {

        ASSERT(fill_it->second.isMap());
        parseInheritMap(fill_.get(), fill_it->second);
        eckit::Log::debug<LibMir>() << "MIRConfiguration fill: " << *fill_ << std::endl;

    }


    eckit::Log::info() << "done" << std::endl;
}


void MIRConfiguration::parseInheritMap(InheritParametrisation* who, const eckit::ValueMap& map) const {
    for (eckit::ValueMap::const_iterator i = map.begin(); i != map.end(); ++i) {
        std::string empty;

        if (i->first == fillKey_) {

            // handled separately in fill_

        } else if (i->second.isMap()) {

            std::vector<long> ids;
            std::vector<std::string> keys;

            if (string_contains_paramIds(i->first, ids)) {

                InheritParametrisation* me = new InheritParametrisation(who, ids);
                parseInheritMap(me, i->second);
                who->child(me);

            } else if (string_contains_keys(i->first, keys)) {

                ASSERT(keys.size() <= 2);
                InheritParametrisation* me = new InheritParametrisation(who,
                    keys.size()>0? keys[0] : empty,
                    keys.size()>1? keys[1] : empty );
                parseInheritMap(me, i->second);
                who->child(me);

            }

        } else {

            if (!(who->has(i->first))) {
                who->set(i->first, std::string(i->second));
            }

        }
    }
}


void MIRConfiguration::print(std::ostream& out) const {
    out << "MIRConfiguration[...]";
}


const param::MIRParametrisation* MIRConfiguration::lookup(const long& paramId, const param::MIRParametrisation& metadata) const {
    param::SimpleParametrisation* param = new param::SimpleParametrisation();
    ASSERT(param);

    // inherit from most-specific paramId/metadata individual and its parents
    root_->pick(paramId, metadata).inherit(*param);

    // inherit recursively from a "filling" key
    std::string fillValue;
    size_t check = 0;
    while (param->get(fillKey_, fillValue)) {
        param->clear(fillKey_);
        ASSERT(check++ < 50);
        fill_->pick(fillKey_, fillValue).inherit(*param);
    }

    return param;
}


const param::MIRParametrisation* MIRConfiguration::lookupDefaults() const {
    static param::SimpleParametrisation empty;
    return lookup(0, empty);
}


}  // namespace config
}  // namespace mir

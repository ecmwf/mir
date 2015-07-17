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

#include "mir/param/JSONConfiguration.h"


#include <iostream>
#include <limits>
#include "eckit/filesystem/PathName.h"
#include "eckit/parser/JSONParser.h"
#include "mir/util/Parser.h"
#include "eckit/parser/Tokenizer.h"


namespace mir {
namespace param {


JSONConfiguration::JSONConfiguration(const eckit::PathName &path, char separator):
    separator_(separator) {
    std::ifstream in(path);
    eckit::JSONParser parser(in);
    root_ = parser.parse();
    ASSERT(!in.bad());
}

JSONConfiguration::JSONConfiguration(std::istream &in, char separator):
    separator_(separator) {
    eckit::JSONParser parser(in);
    root_ = parser.parse();
    ASSERT(!in.bad());
}

JSONConfiguration::JSONConfiguration(const JSONConfiguration &other, const std::string &path):
    root_(other.lookUp(path)),
    separator_(other.separator_) {
}

JSONConfiguration::~JSONConfiguration() {
}

void JSONConfiguration::print(std::ostream &out) const {
    out << "JSONConfiguration[root=";
    out << root_;
    out << "]";
}

eckit::Value JSONConfiguration::lookUp(const std::string &s, bool &found) const {

    eckit::Tokenizer parse(separator_);
    std::vector<std::string> path;
    parse(s, path);

    eckit::Value result = root_;

    std::cout << "JSONConfiguration::lookUp root=" << result << std::endl;
    for (size_t i = 0; i < path.size(); i++) {
        const std::string &key = path[i];
        if (!result.contains(key)) {
            found = false;
            return result;
        }
        std::cout << "JSONConfiguration::lookUp key=" << key  << std::endl;

        result = result[key];
        std::cout << "JSONConfiguration::lookUp key=" << result  << std::endl;

    }

    found = true;
    return result;
}

void JSONConfiguration::set(const std::vector<std::string> &path, size_t i, eckit::Value &root, const eckit::Value &value)  {
    if (root.shared()) {
        std::cout << "Clone " << root << std::endl;
        root = root.clone();
    }

    if (i + 1 == path.size()) {
        std::cout << i << " SET " << path[i] << " to " << value << std::endl;
        root[path[i]] = value;
        return;
    }

    if (!root.contains(path[i])) {
        std::cout << i << " NEW " << path[i]  << std::endl;
        root[path[i]] = eckit::Value::makeMap();
    }

    eckit::Value &r = root.element(path[i]);
    set(path, i + 1, r, value);
}

void JSONConfiguration::set(const std::string &s, const eckit::Value &value)  {

    std::cout << "---- " << s << " => " << value << std::endl;

    eckit::Tokenizer parse(separator_);
    std::vector<std::string> path;
    parse(s, path);

    set(path, 0, root_, value);
}

void JSONConfiguration::set(const std::string &s, long value)  {
    set(s, eckit::Value(value));
}


void JSONConfiguration::set(const std::string &s, const char* value)  {
    set(s, eckit::Value(value));
}


void JSONConfiguration::set(const std::string &s, const std::string& value)  {
    set(s, eckit::Value(value));
}

void JSONConfiguration::set(const std::string &s, double value)  {
    set(s, eckit::Value(value));
}

void JSONConfiguration::set(const std::string &s, bool value)  {
    set(s, eckit::Value(value));
}

eckit::Value JSONConfiguration::lookUp(const std::string &name) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    ASSERT(found);
    return v;
}


bool JSONConfiguration::has(const std::string &name) const {
    bool found = false;
    lookUp(name, found);
    return found;
}

bool JSONConfiguration::get(const std::string &name, std::string &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        value = std::string(v);
    }
    return found;
}

bool JSONConfiguration::get(const std::string &name, bool &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        value = v;
    }
    return found;
}

bool JSONConfiguration::get(const std::string &name, long &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        value = long(v);
    }
    return found;
}

bool JSONConfiguration::get(const std::string &name, double &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        value = v;
    }
    return found;
}

bool JSONConfiguration::get(const std::string &name, std::vector<long> &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        ASSERT(v.isList());
        value.clear();
        int i = 0;
        while (v.contains(i)) {
            value.push_back(v[i]);
            i++;
        }
    }
    return found;
}

bool JSONConfiguration::get(const std::string &name, std::vector<double> &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        ASSERT(v.isList());
        value.clear();
        int i = 0;
        while (v.contains(i)) {
            value.push_back(v[i]);
            i++;
        }
    }
    return found;
}

bool JSONConfiguration::get(const std::string &name, size_t &value) const {
    bool found = false;
    eckit::Value v = lookUp(name, found);
    if (found) {
        value = long(v);
    }
    return found;
}

}  // namespace param
}  // namespace mir


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
    size_t len = s.size();
    size_t j = 0;
    eckit::Value result = root_;

    std::cout << "JSONConfiguration::lookUp root=" << result << std::endl;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == separator_) {
            std::string key = s.substr(j, i);
            if (!result.contains(key)) {
                found = false;
                return result;
            }

            std::cout << "JSONConfiguration::lookUp key='" << key << "'" << std::endl;
            result = result[key];
            std::cout << "JSONConfiguration::lookUp result=" << result << std::endl;

            j = i + 1;
        }
    }
    if (j < len) {
        std::string key = s.substr(j);
        if (!result.contains(key)) {
            found = false;
            return result;
        }
        std::cout << "JSONConfiguration::lookUp key='" << key << "'" << std::endl;
        result = result[key];
        std::cout << "JSONConfiguration::lookUp result=" << result << std::endl;

    }
    found = true;
    return result;
}


eckit::Value JSONConfiguration::lookUp(const std::string& name) const {
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


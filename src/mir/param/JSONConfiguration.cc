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


JSONConfiguration::~JSONConfiguration() {
}

void JSONConfiguration::print(std::ostream &out) const {
    out << "JSONConfiguration[root=";
    out << root_;
    out << "]";
}

bool JSONConfiguration::has(const std::string &name) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, std::string &value) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, bool &value) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, long &value) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, double &value) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, std::vector<long> &value) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, std::vector<double> &value) const {
    NOTIMP;
}

bool JSONConfiguration::get(const std::string &name, size_t &value) const {
    NOTIMP;
}

}  // namespace param
}  // namespace mir


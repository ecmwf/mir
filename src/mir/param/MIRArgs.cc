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

#include "mir/param/MIRArgs.h"

#include <iostream>

#include "eckit/runtime/Context.h"
#include "eckit/parser/Tokenizer.h"

namespace mir {
namespace param {


MIRArgs::MIRArgs(usage_proc usage, int args_count) {
    eckit::Context& ctx = eckit::Context::instance();
    const std::string& tool = ctx.runName();
    size_t argc = ctx.argc();

    // mir::input::GribFileInput input(ctx.argv(argc - 2));
    // mir::output::GribFileOutput output(ctx.argv(argc - 1));

    eckit::Tokenizer parse("=");
    for (size_t i = 1; i < argc; i++) {

        std::string a = ctx.argv(i);
        if (a.size() > 2 && a[0] == '-' && a[1] == '-') {
            std::vector<std::string> v;
            parse(a.substr(2), v);
            ASSERT(v.size() == 2) ;
            set(v[0], v[1]);
            std::cout << v[0] << " - " <<v[1] << std::endl;
            keys_.insert(v[0]);
        } else {
            args_.push_back(a);
        }
    }

    if (args_count >= 0) {
        if (args_.size() != args_count) {
            usage(tool);
            ::exit(1);
        }
    }
}


MIRArgs::~MIRArgs() {
}


void MIRArgs::print(std::ostream& out) const {
    out << "MIRArgs[";
    SimpleParametrisation::print(out);
    out << "]";
}

const std::set<std::string>& MIRArgs::keys() const {
    return keys_;
}

const std::vector<std::string>& MIRArgs::args() const {
    return args_;
}

const std::string& MIRArgs::args(size_t i) const {
    ASSERT(i < args_.size());
    return args_[i];
}

bool MIRArgs::has(const std::string& name) const {
    return SimpleParametrisation::has(name);
}

bool MIRArgs::get(const std::string& name, std::string& value) const {
    return SimpleParametrisation::get(name, value);
}

bool MIRArgs::get(const std::string& name, bool& value) const {
    return SimpleParametrisation::get(name, value);
}

bool MIRArgs::get(const std::string& name, long& value) const {
    return SimpleParametrisation::get(name, value);
}

bool MIRArgs::get(const std::string& name, double& value) const {
    return SimpleParametrisation::get(name, value);
}

bool MIRArgs::get(const std::string& name, std::vector<long>& value) const {
    return SimpleParametrisation::get(name, value);
}

bool MIRArgs::get(const std::string& name, std::vector<double>& value) const {
    return SimpleParametrisation::get(name, value);
}


}  // namespace param
}  // namespace mir


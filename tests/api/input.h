// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir::api::tests {


using input::MIRInput;
using input_t = std::unique_ptr<MIRInput>;


/// Input built as the mir tool does, from a path and the --input option
inline input_t build(const std::string& path, const std::string& input = "") {
    param::SimpleParametrisation param;
    if (!input.empty()) {
        param.set("input", input);
    }
    return input_t(input::MIRInputFactory::build(path, param));
}


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


inline void write_file(const std::string& path, const std::string& contents) {
    std::ofstream out(path);
    out << contents;
}


inline size_t number_of_values(const data::MIRField& field) {
    repres::RepresentationHandle repres(field.representation());
    return repres->numberOfValues();
}


}  // namespace mir::api::tests

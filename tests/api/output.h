// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/input/RawInput.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::api::tests {


using output::MIROutput;
using output_t = std::unique_ptr<MIROutput>;


/// Input of a field on a grid (by its gridSpec), values by point index, with metadata
struct GridInput {
    explicit GridInput(const std::string& gridspec, size_t dimensions = 1) : grid(gridspec), metadata(grid) {
        auto n = grid.grid().size();
        for (size_t d = 0; d < dimensions; ++d) {
            for (size_t i = 0; i < n; ++i) {
                values.push_back(static_cast<double>(i + d * 100));
            }
        }

        if (dimensions > 1) {
            metadata.set("dimensions", static_cast<long>(dimensions));
        }

        input = std::make_unique<input::RawInput>(values.data(), n, metadata);
        ASSERT(input->next());
    }

    input::MIRInput& operator*() { return *input; }

    param::GridSpecParametrisation grid;
    param::RuntimeParametrisation metadata;
    std::vector<double> values;
    std::unique_ptr<input::MIRInput> input;
};


template <typename T>
std::string to_string(const T& p) {
    std::ostringstream str;
    str << p;
    return str.str();
}


inline std::vector<std::string> read_lines(const std::string& path) {
    std::ifstream in(path);
    std::vector<std::string> lines;
    for (std::string line; std::getline(in, line);) {
        lines.push_back(line);
    }
    return lines;
}


inline std::string read_bytes(const std::string& path, size_t n) {
    std::ifstream in(path, std::ios::binary);
    std::string bytes(n, '\0');
    in.read(&bytes[0], static_cast<std::streamsize>(n));
    return bytes;
}


}  // namespace mir::api::tests

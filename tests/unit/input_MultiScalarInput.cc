/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <memory>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/input/DummyInput.h"
#include "mir/input/MultiScalarInput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


CASE("MultiScalarInput") {
    static const param::DefaultParametrisation defaults;

    SECTION("next(), dimensions()") {
        const std::vector<size_t> _numberOfFields{1, 2, 3, 4, 5};
        const std::vector<size_t> _multiScalar{1, 2, 3};

        for (size_t numberOfFields : _numberOfFields) {
            for (size_t multiScalar : _multiScalar) {
                auto numberOfNext = (numberOfFields - 1) / multiScalar + 1;
                ASSERT(numberOfNext >= 1);

                Log::info() << "Test numberOfFields/multiScalar = numberOfNext => " << numberOfFields << " / "
                            << multiScalar << " = " << numberOfNext << std::endl;

                auto multi = new input::MultiScalarInput();
                for (size_t i = 0; i < numberOfFields; ++i) {
                    multi->appendScalarInput(new input::DummyInput(defaults));
                }

                size_t dim  = 0;
                size_t next = 0;
                for (std::unique_ptr<input::MIRInput> input(multi); input->next(); next++) {
                    dim += input->dimensions();
                    ASSERT(dim <= numberOfFields);

                    Log::info() << "\tnext() => dimensions() = " << input->dimensions() << std::endl;

                    auto Ndim = dim == numberOfFields ? (numberOfFields - 1) % multiScalar + 1 : multiScalar;
                    EXPECT(Ndim == dim);
                }

                EXPECT(numberOfFields == dim);
                EXPECT(numberOfNext == next);
            }
        }
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

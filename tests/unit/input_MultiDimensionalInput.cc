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
#include "mir/input/MultiDimensionalInput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace tests {
namespace unit {


struct TestingInput : input::MIRInput {
    const param::MIRParametrisation& parametrisation(size_t) const override { NOTIMP; }
    bool sameAs(const MIRInput&) const override { return false; }
    data::MIRField field() const override { NOTIMP; }
    void print(std::ostream&) const override { NOTIMP; }
};


struct TestingMultiDimensionalInput final : TestingInput {
    TestingMultiDimensionalInput(size_t Nfields, size_t Ndim) {
        for (size_t n = Nfields; n > 0; n -= Ndim) {
            if (n < Ndim) {
                dims_.insert(dims_.begin(), n);
                break;
            }
            dims_.push_back(Ndim);
        }
        dims_.push_back(0);
    }

private:
    bool next() override {
        dims_.pop_back();
        return !dims_.empty();
    }

    size_t dimensions() const override {
        ASSERT(!dims_.empty());
        return dims_.back();
    }

    std::vector<size_t> dims_;
};


struct TestingSingleDimensionalInput final : TestingInput {
    TestingSingleDimensionalInput(size_t Nfields) : Nfields_(Nfields + 1) {}

private:
    bool next() override { return --Nfields_ != 0; }
    size_t dimensions() const override { return 1; }
    size_t Nfields_;
};


CASE("MultiDimensionalInput") {


    SECTION("MultiDimensionalInput::next(), dimensions()") {
        const std::vector<size_t> _numberOfFields{1, 2, 3, 4, 5};
        const std::vector<size_t> _numberOfDimensions{1, 2, 3};

        for (size_t numberOfFields : _numberOfFields) {
            for (size_t numberOfDimensions : _numberOfDimensions) {
                auto numberOfNext = (numberOfFields - 1) / numberOfDimensions + 1;
                ASSERT(numberOfNext >= 1);

                Log::info() << "Test numberOfFields/numberOfDimensions = numberOfNext => " << numberOfFields << " / "
                            << numberOfDimensions << " = " << numberOfNext << std::endl;

                std::unique_ptr<input::MIRInput> input(
                    new TestingMultiDimensionalInput(numberOfFields, numberOfDimensions));

                size_t Nfields = 0;
                size_t Nnext   = 0;
                for (; input->next(); Nnext++) {
                    auto Ndims = input->dimensions();
                    Log::info() << "\tnext() => dimensions() = " << Ndims << std::endl;

                    Nfields += Ndims;

                    auto numberOfDims =
                        Nfields == numberOfFields ? (numberOfFields - 1) % numberOfDimensions + 1 : numberOfDimensions;

                    EXPECT(numberOfFields >= Nfields);
                    EXPECT(numberOfDims == Ndims);
                }

                EXPECT(numberOfFields == Nfields);
                EXPECT(numberOfNext == Nnext);
            }
        }
    }


    SECTION("MultiDimensionalInput empty") {
        std::unique_ptr<input::MIRInput> input(new input::MultiDimensionalInput);
        EXPECT(!input->next());
    }


    SECTION("MultiDimensionalInput::append() different-sized fields") {
        std::unique_ptr<input::MIRInput> input([]() {
            auto* multi = new input::MultiDimensionalInput;
            ASSERT(multi != nullptr);

            multi->append(new TestingSingleDimensionalInput(4));
            multi->append(new TestingSingleDimensionalInput(2));
            multi->append(new TestingSingleDimensionalInput(4));

            return multi;
        }());

        EXPECT(input->next());
        EXPECT(input->dimensions() == 3);

        EXPECT(input->next());
        EXPECT(input->dimensions() == 3);

        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);

        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);

        EXPECT(!input->next());
    }


    SECTION("MultiDimensionalInput::append() include empty-sized fields") {
        std::unique_ptr<input::MIRInput> input([]() {
            auto* multi = new input::MultiDimensionalInput;
            ASSERT(multi != nullptr);

            multi->append(new TestingSingleDimensionalInput(2));
            multi->append(new TestingSingleDimensionalInput(0));
            multi->append(new TestingSingleDimensionalInput(3));

            return multi;
        }());

        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);

        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);

        EXPECT(input->next());
        EXPECT(input->dimensions() == 1);

        EXPECT(!input->next());
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

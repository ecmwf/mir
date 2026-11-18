// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/MultiDimensionalInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/ValueMap.h"

#include "input.h"


namespace mir::api::tests {


CASE("MultiDimensionalInput") {
    struct TestingInput : MIRInput {
        const param::MIRParametrisation& parametrisation(size_t /*which*/) const override { NOTIMP; }
        bool sameAs(const MIRInput& /*unused*/) const override { return false; }
        data::MIRField field() const override { NOTIMP; }
        void print(std::ostream& /*unused*/) const override { NOTIMP; }
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
        explicit TestingSingleDimensionalInput(size_t Nfields) : Nfields_(Nfields + 1) {}

    private:
        bool next() override { return --Nfields_ != 0; }
        size_t dimensions() const override { return 1; }
        size_t Nfields_;
    };


    SECTION("next(), dimensions()") {
        const std::vector<size_t> _numberOfFields{1, 2, 3, 4, 5};
        const std::vector<size_t> _numberOfDimensions{1, 2, 3};

        for (const auto& numberOfFields : _numberOfFields) {
            for (const auto& numberOfDimensions : _numberOfDimensions) {
                auto numberOfNext = (numberOfFields - 1) / numberOfDimensions + 1;
                ASSERT(numberOfNext >= 1);

                input_t input(new TestingMultiDimensionalInput(numberOfFields, numberOfDimensions));

                size_t Nfields = 0;
                size_t Nnext   = 0;
                for (; input->next(); Nnext++) {
                    auto Ndims = input->dimensions();
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


    SECTION("empty") {
        input_t input(new input::MultiDimensionalInput);
        EXPECT(!input->next());
    }


    SECTION("append() different-sized fields") {
        auto* multi = new input::MultiDimensionalInput;
        input_t input(multi);
        multi->append(new TestingSingleDimensionalInput(4));
        multi->append(new TestingSingleDimensionalInput(2));
        multi->append(new TestingSingleDimensionalInput(4));

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


    SECTION("append() include empty-sized fields") {
        auto* multi = new input::MultiDimensionalInput;
        input_t input(multi);
        multi->append(new TestingSingleDimensionalInput(2));
        multi->append(new TestingSingleDimensionalInput(0));
        multi->append(new TestingSingleDimensionalInput(3));

        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 2);
        EXPECT(input->next());
        EXPECT(input->dimensions() == 1);
        EXPECT(!input->next());
    }


    SECTION("GRIB components") {
        auto make = []() {
            auto* multi = new input::MultiDimensionalInput;
            multi->append(new input::GribFileInput("tiny-3x3.grib", 0, 2));
            multi->append(new input::GribFileInput("tiny-3x3.grib", 1, 2));
            return input_t(multi);
        };

        auto in = make();
        EXPECT(in->next());
        EXPECT_EQUAL(in->dimensions(), 2);

        auto field = in->field();
        EXPECT_EQUAL(field.dimensions(), 2);
        EXPECT_EQUAL(field.values(1).size(), 9);

        long edition = 0;
        EXPECT(in->parametrisation(1).get("edition", edition) && edition == 2);
        EXPECT(in->gribHandle(1) != nullptr);
        EXPECT_THROWS(in->parametrisation(2));

        EXPECT_NO_THROW(in->setAuxiliaryInformation(util::ValueMap()));

        EXPECT(in->sameAs(*make()));
        EXPECT_NOT(in->sameAs(*build("tiny-3x3.grib")));

        auto other = std::make_unique<input::MultiDimensionalInput>();
        other->append(new input::GribFileInput("tiny-3x3.grib", 1, 2));
        other->append(new input::GribFileInput("tiny-3x3.grib", 0, 2));
        EXPECT_NOT(in->sameAs(*other));

        EXPECT(to_string(*in).find("MultiDimensionalInput[GribFileInput[") == 0);
    }
}


}  // namespace mir::api::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

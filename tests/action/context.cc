// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <memory>
#include <sstream>
#include <string>

#include "eckit/exception/Exceptions.h"

#include "mir/input/MIRInput.h"
#include "mir/param/SimpleParametrisation.h"

#include "action.h"


namespace mir::action::tests {


/// Context extension, counting its copies
struct TestExtension : context::Extension {
    explicit TestExtension(int v, int n = 0) : value(v), copies(n) {}

    TestExtension* clone() const override { return new TestExtension(value, copies + 1); }
    void print(std::ostream& out) const override { out << "TestExtension[" << value << "]"; }

    int value;
    int copies;
};


template <typename T>
std::string to_str(const T& x) {
    std::ostringstream out;
    out << x;
    return out.str();
}


CASE("Context, of a field") {
    util::MIRStatistics statistics;
    auto f = field("{grid: [10, 10]}", [](double lat, double) { return lat; }, 2);

    context::Context ctx(f, statistics);

    EXPECT(ctx.isField());
    EXPECT(!ctx.isScalar());
    EXPECT(!ctx.isExtension());
    EXPECT(&ctx.statistics() == &statistics);

    EXPECT_EQUAL(ctx.field().dimensions(), 2);
    EXPECT(ctx.field().values(0) == f.values(0));
    EXPECT(to_str(ctx).rfind("Context[content=FieldContent[field=", 0) == 0);

    // no scalar, no extension
    try {
        ctx.scalar();
        EXPECT(false);
    }
    catch (const eckit::Exception& e) {
        EXPECT(std::string(e.what()).find("Cannot get scalar from ") != std::string::npos);
    }
    EXPECT_THROWS(ctx.extension());

    SECTION("select") {
        ctx.select(1);

        EXPECT_EQUAL(ctx.field().dimensions(), 1);
        EXPECT(ctx.field().values(0) == f.values(1));
    }

    SECTION("substitute the field") {
        auto other = field("{grid: O8}", [](double, double) { return 1.; });
        ctx.field(other);

        EXPECT(ctx.isField());
        EXPECT_EQUAL(ctx.field().dimensions(), 1);
        EXPECT_EQUAL(ctx.field().values(0).size(), 544);
    }

    SECTION("copy (locking the original), push and pop: independent copies") {
        context::Context copy(ctx);
        copy.select(0);

        EXPECT_EQUAL(copy.field().dimensions(), 1);
        EXPECT_EQUAL(ctx.field().dimensions(), 2);

        auto& pushed = ctx.push();
        pushed.scalar(3.);

        EXPECT(ctx.isField());

        auto popped = ctx.pop();

        EXPECT(popped.isScalar());
        EXPECT_EQUAL(popped.scalar(), 3.);
        EXPECT_THROWS(ctx.pop());  // empty stack
    }
}


CASE("Context, of a scalar") {
    util::MIRStatistics statistics;
    auto f = field("{grid: [10, 10]}");

    context::Context ctx(f, statistics);
    ctx.scalar(2.5);

    EXPECT(ctx.isScalar());
    EXPECT(!ctx.isField());
    EXPECT(!ctx.isExtension());
    EXPECT_EQUAL(ctx.scalar(), 2.5);
    EXPECT_EQUAL(to_str(ctx), "Context[content=ScalarContent[value=2.5]]");

    EXPECT_THROWS(ctx.field());
    EXPECT_THROWS(ctx.extension());
}


CASE("Context, of an extension") {
    util::MIRStatistics statistics;
    auto f = field("{grid: [10, 10]}");

    context::Context ctx(f, statistics);
    ctx.extension(new TestExtension(42));

    EXPECT(ctx.isExtension());
    EXPECT(!ctx.isField());
    EXPECT(!ctx.isScalar());
    EXPECT_EQUAL(to_str(ctx), "Context[content=ExtensionContent[TestExtension[42]]]");

    const auto* e = dynamic_cast<const TestExtension*>(&ctx.extension());

    EXPECT(e != nullptr);
    EXPECT_EQUAL(e->value, 42);
    EXPECT_EQUAL(e->copies, 0);

    EXPECT_THROWS(ctx.field());
    EXPECT_THROWS(ctx.scalar());

    // copies clone the extension
    context::Context copy(ctx);
    const auto* c = dynamic_cast<const TestExtension*>(&copy.extension());

    EXPECT(c != nullptr && c != e);
    EXPECT_EQUAL(c->value, 42);
    EXPECT_EQUAL(c->copies, 1);
}


CASE("Context, of an input") {
    util::MIRStatistics statistics;

    param::SimpleParametrisation param;
    param.set("input", "{artificialInput: gridspec, gridSpec: {grid: O8}}");

    std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build("gridspec", param));
    EXPECT(input->next());

    context::Context ctx(*input, statistics);

    // the field is from the input, on demand
    EXPECT(&ctx.input() == input.get());
    EXPECT(!ctx.isField());
    EXPECT_EQUAL(to_str(ctx), "Context[content=null]");

    EXPECT_EQUAL(ctx.field().values(0).size(), 544);
    EXPECT(ctx.isField());
}


CASE("Context, empty") {
    context::Context ctx;

    EXPECT(!ctx.isField());
    EXPECT(!ctx.isScalar());
    EXPECT(!ctx.isExtension());
    EXPECT_EQUAL(to_str(ctx), "Context[content=null]");
    EXPECT_EQUAL(to_str(ctx.input()), "MissingInput[]");

    // the (missing) input has no field
    EXPECT_THROWS(ctx.field());
    EXPECT_THROWS(ctx.scalar());
    EXPECT_THROWS(ctx.extension());
}


}  // namespace mir::action::tests


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

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


#include <ostream>
#include <vector>

#include "eckit/config/Configuration.h"
#include "eckit/testing/Test.h"

#include "mir/config/LibMir.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/Wind.h"

#define EXPECTV(a)                                \
    log << "\tEXPECT(" << #a << ")" << std::endl; \
    EXPECT(a)


namespace mir::tests::unit {


struct FakeInput : public param::RuntimeParametrisation {
    explicit FakeInput(long paramId) : param::RuntimeParametrisation(simple_) { simple_.set("paramId", paramId); }
    long paramId() const {
        long id = 0;
        ASSERT(simple_.get("paramId", id));
        return id;
    }

private:
    param::SimpleParametrisation simple_;
};


CASE("MIR-324") {
    using util::Wind;
    auto& log = Log::debug();


    auto PARAMID_U = LibMir::instance().configuration().getLong("parameter-id-u", 131);
    auto PARAMID_V = LibMir::instance().configuration().getLong("parameter-id-v", 132);

    const std::vector<long> _table{0, 129, 171, 200};
    const std::vector<long> _user_u{0, 1, PARAMID_U, 999, 999999};
    const std::vector<long> _user_v{0, 1, PARAMID_V, 999, 999999};


    SECTION("Wind: u/v paramId from vorticity/divergence") {
        for (long table : _table) {
            for (long user_u : _user_u) {
                for (long user_v : _user_v) {

                    FakeInput input(138 + table * 1000);

                    if (user_u != 0) {
                        input.set("paramId.u", user_u);
                    }

                    if (user_v != 0) {
                        input.set("paramId.v", user_v);
                    }

                    long u = 0;
                    long v = 0;
                    Wind::paramIds(input, u, v);

                    static size_t c = 1;
                    log << "Test " << c++ << ":" << "\n\t   input paramId = " << input.paramId() << "\n\t + paramId.u "
                        << (user_u != 0 ? "(set)" : "(not set)") << " = " << user_u << "\n\t + paramId.v "
                        << (user_v != 0 ? "(set)" : "(not set)") << " = " << user_v << "\n\t = " << "\n\t   u = " << u
                        << "\n\t   v = " << v << std::endl;

                    EXPECTV(u == (user_u != 0 ? user_u : PARAMID_U + table * 1000));
                    EXPECTV(v == (user_v != 0 ? user_v : PARAMID_V + table * 1000));
                }
            }
        }
    }
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

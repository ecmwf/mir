/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>
#include <string>
#include <vector>

#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Wind.h"

#define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)


namespace mir {
namespace tests {
namespace unit {


struct FakeInput : public param::RuntimeParametrisation {
    FakeInput(size_t paramId) : param::RuntimeParametrisation(simple_) {
        simple_.set("paramId", paramId);
    }
    size_t paramId() const {
        long id = 0;
        ASSERT(simple_.get("paramId", id));
        return size_t(id);
    }
private:
    param::SimpleParametrisation simple_;
};


CASE("MIR-324") {
    using util::Wind;
    auto& log = eckit::Log::debug<LibMir>();


    static const long PARAMID_U = LibMir::instance().configuration().getLong("parameter-id-u", 131);
    static const long PARAMID_V = LibMir::instance().configuration().getLong("parameter-id-v", 132);


    SECTION("Wind: u/v paramId from vorticity/divergence") {
        for (size_t table : std::vector<size_t>{0, 129, 171, 200}) {
            for (long user_u : std::vector<long>{ 0, 1, PARAMID_U, 999, 999999 }) {
                for (long user_v : std::vector<long>{ 0, 1, PARAMID_V, 999, 999999 }) {

                    FakeInput input(138 + table * 1000);

                    if (user_u) {
                        input.set("paramId.u", user_u);
                    }

                    if (user_v) {
                        input.set("paramId.v", user_v);
                    }

                    size_t u = 0;
                    size_t v = 0;
                    Wind::paramIds(input, u, v);

                    static size_t c = 1;
                    log << "Test " << c++ << ":"
                        << "\n\t   input paramId = " << input.paramId()
                        << "\n\t + paramId.u " << (user_u ? "(set)" : "(not set)") << " = " << user_u
                        << "\n\t + paramId.v " << (user_v ? "(set)" : "(not set)") << " = " << user_v
                        << "\n\t = "
                        << "\n\t   u = " << u
                        << "\n\t   v = " << v
                        << std::endl;

                    EXPECTV(u == (user_u ? size_t(user_u) : PARAMID_U + table * 1000));
                    EXPECTV(v == (user_v ? size_t(user_v) : PARAMID_V + table * 1000));
                }
            }
        }
    }


    SECTION("Wind::isInputWind") {
        for (auto paramid : std::vector<long>{ 0, 1, PARAMID_U, PARAMID_V, 999 }) {

            size_t id(paramid);
            FakeInput input(id);

            bool inputIsWind = (paramid == PARAMID_U || paramid == PARAMID_V);

            EXPECTV(inputIsWind == Wind::isInputWind(input));
            EXPECTV(inputIsWind == Wind::isOutputWind(input));
        }
    }


    SECTION("Wind::isOutputWind") {

        FakeInput user(0);
        ASSERT(!Wind::isInputWind(user));

        for (bool vod2uv : {false, true}) {
            for (bool wind : {false, true}) {

                vod2uv ? user.set("vod2uv", true) : user.clear("vod2uv");
                wind ? user.set("wind", true) : user.clear("wind");

                EXPECTV(Wind::isOutputWind(user) == (vod2uv || wind));
            }
        }
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char **argv) {
    return eckit::testing::run_tests(argc, argv, true);
}


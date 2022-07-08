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

#include "eckit/testing/Test.h"

#include "mir/util/Log.h"
#include "mir/util/SpectralOrder.h"


namespace mir {
namespace tests {
namespace unit {


CASE("test_spectral_order") {
    using util::SpectralOrder;
    using util::SpectralOrderFactory;

    // Cases from hsh2gg.F and gaussgr (ifs-scripts/production suite)

    const long order1_cases[][2] = {{15999, 8000},
                                    {7999, 4000},
                                    {3999, 2000},
                                    {3199, 1600},
                                    {2559, 1280},
                                    {2047, 1024},
                                    {1599, 800},
                                    {1279, 640},
                                    {1023, 512},
                                    {799, 400},
                                    {639, 320},
                                    {511, 256},
                                    {399, 200},
                                    {319, 160},
                                    {255, 128},
                                    {191, 96},
                                    {159, 80},
                                    {95, 48},
                                    {63, 32},
                                    {31, 16},
                                    {
                                        0,
                                    }};

    // (*1) gaussgr: introduced in ifs-scripts commit cd22bde5, works if N=1365
    const long order2_cases[][2] = {{1706, 1280},
                                    // (*1) {1364, 1024},
                                    {853, 640},
                                    {341, 256},
                                    {213, 160},
                                    {170, 128},
                                    {106, 80},
                                    {63, 48},
                                    {42, 32},
                                    {21, 16},
                                    {
                                        0,
                                    }};

    const long order3_cases[][2] = {{7999, 8000},
                                    {3999, 4000},
                                    {2559, 2560},
                                    {2047, 2048},
                                    {1999, 2000},
                                    {1599, 1600},
                                    {1279, 1280},
                                    {1023, 1024},
                                    {911, 912},
                                    {799, 800},
                                    {639, 640},
                                    {511, 512},
                                    {399, 400},
                                    {319, 320},
                                    {255, 256},
                                    {199, 200},
                                    {191, 192},
                                    {159, 160},
                                    {95, 96},
                                    {79, 80},
                                    {63, 64},
                                    {
                                        0,
                                    }};

    const long unknown_cases[][2] = {{213, 128},
                                     {
                                         0,
                                     }};


    std::unique_ptr<SpectralOrder> order1(SpectralOrderFactory::build("linear"));

    for (long Tref, Nref, i = 0; (Tref = order1_cases[i][0]) != 0; ++i) {
        Nref = order1_cases[i][1];

        long N = order1->getGaussianNumberFromTruncation(Tref);
        long T = order1->getTruncationFromGaussianNumber(Nref);

        Log::debug() << "N(T=" << T << ")|linear = " << N << ", expecting N=" << Nref << std::endl;
        EXPECT(N == Nref);

        Log::debug() << "T(N=" << N << ")|linear = " << T << ", expecting T=" << Tref << std::endl;
        EXPECT(T == Tref);
    }


    std::unique_ptr<SpectralOrder> order2(SpectralOrderFactory::build("quadratic"));

    for (long Tref, Nref, i = 0; (Tref = order2_cases[i][0]) != 0; ++i) {
        Nref = order2_cases[i][1];

        long N = order2->getGaussianNumberFromTruncation(Tref);
        long T = order2->getTruncationFromGaussianNumber(Nref);

        Log::debug() << "N(T=" << T << ")|quadratic = " << N << ", expecting N=" << Nref << std::endl;
        EXPECT(N == Nref);

        Log::debug() << "T(N=" << N << ")|quadratic = " << T << ", expecting T=" << Tref << std::endl;
        EXPECT(T == Tref);
    }


    std::unique_ptr<SpectralOrder> order3(SpectralOrderFactory::build("cubic"));

    for (long Tref, Nref, i = 0; (Tref = order3_cases[i][0]) != 0; ++i) {
        Nref = order3_cases[i][1];

        long N = order3->getGaussianNumberFromTruncation(Tref);
        long T = order3->getTruncationFromGaussianNumber(Nref);

        Log::debug() << "N(T=" << T << ")|cubic = " << N << ", expecting N=" << Nref << std::endl;
        EXPECT(N == Nref);

        Log::debug() << "T(N=" << N << ")|cubic = " << T << ", expecting T=" << Tref << std::endl;
        EXPECT(T == Tref);
    }

    for (long T, i = 0; (T = unknown_cases[i][0]) != 0; ++i) {
        const long N1 = order1->getGaussianNumberFromTruncation(T);
        const long N2 = order2->getGaussianNumberFromTruncation(T);
        EXPECT(N1 != unknown_cases[i][1]);
        EXPECT(N2 != unknown_cases[i][1]);

        const long T1 = order1->getTruncationFromGaussianNumber(unknown_cases[i][1]);
        const long T2 = order2->getTruncationFromGaussianNumber(unknown_cases[i][1]);
        EXPECT(T1 != unknown_cases[i][0]);
        EXPECT(T2 != unknown_cases[i][0]);
    }
}


}  // namespace unit
}  // namespace tests
}  // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace tests {
namespace unit {


CASE("BoundingBox") {

    using util::BoundingBox;

    auto& log = eckit::Log::info();
    auto old = log.precision(16);

    const std::vector< BoundingBox > boxes {
        {  90,             0,          -90,           360          },
        {  90,             0,          -90,           720          },
        {  90,          -360,          -90,           360          },
        {  90,          -180,          -90,           180          },
        {  90,           180,          -90,          -180          },
        {  90,             0,          -90,           356          },
        {  90,             0,          -90,           358          },
        {  90,             0,          -90,           359.5        },
        {  90,             0,          -90,           359          },
        {  90,             0,          -90,           359.999      },
        {  90,          -180,          -90,           179.99       },
        {  90,             0,          -90,           359.99       },
        {  90,             0,          -90,           359.9        },
        {  90,             0,          -90,           359.6489     },
        {  90,          -350,          -10,             9          },
        {  90,            10,          -10,             9          },
        {  89.7626,     -114.8915,     -88.2374,      243.1085     },
        {  89.7626,     -114.8907,     -88.2374,      243.1093     },
        {  89,             1,          -89,           359          },
        {  88,          -178,          -88,           180          },
        {  85,             0,          -90,           357          },
        {  71.8,         -10.66,        34.56,         32.6        },
        {  70.9,         -40.987,       19.73,         40          },
        {  59.9531,       23,           35.0722,       80          },
        {  58.5,          -6.1,         36,            20.7        },
        {  57.9852,      230,           25.0918,      300          },
        {  51.9406,        7.00599,     43.0847,       27.6923     },
        {  51.941,         7.005,       43.084,        27.693      },
        {  43.9281,       91,           21.0152,      143          },
        {  40,            50,          -50,           169.532      },
        {  37.6025,     -114.8915,      27.7626,     -105.188      },
        {  37.6025,     -114.8915,      27.7626,     -105.1875     },
        {  37.6025,     -114.8907,      27.7626,     -105.1875     },
        {  37.6025,     -114.891,       27.7626,     -105.188      },
        {  37.575,      -114.892,       27.803,      -105.187      },
        {  37.5747,      245.109,       27.8032,      254.812      },
        {  36.345879,    113.586968,    35.816463,    114.420328   },
        {  36.34501645,  113.58806225,  35.81244723,  114.41866527 },
        {  35.00112,     112.9995593,   33.990671,    114.0092641  },
        {  34.657355,    113.04832,     34.29085525,  113.74528    },
        {  34.6548026,   113.04894614,  34.2911562,   113.747272   },
        {  27.9,         253,           27.8,         254          },
        {  11.8782,      279,          -49.9727,      325          },
        {   3,             1,            1,             3          },
        {   2.1,           0,            0,             2.1        },
        {   2,             0,            0,             2          },
        {   0,          -350,            0,             9          },
        {   0,          -350,            0,             8          },
        { -10,           -85,          -39,           -56.1        },
        { -10.0176,      275,          -38.9807,      304          },
        { -10.017,       -85,          -38.981,       -56          },
        { -25.0918,      135,          -46.8801,      179          },
    };

    SECTION("intersects") {
//#define EXPECTV(a) log << "\tEXPECT(" << #a <<")" << std::endl; EXPECT(a)
        for (const auto& A : boxes) {
            for (const auto& B : boxes) {

                auto AiB = B;
                auto BiA = A;
                bool commutative = A.intersects(AiB) == B.intersects(BiA);

                static size_t c = 1;
                log << "Test " << c++ << ":" << std::boolalpha
                    << "\n\t" "A=" << A << " (empty? " << A.empty() << ")"
                    << "\n\t" "B=" << B << " (empty? " << B.empty() << ")"
                    << "\n\t" "A intersects B = (empty? " << AiB.empty() << ") = " << AiB
                    << "\n\t" "B intersects A = (empty? " << BiA.empty() << ") = " << BiA
                    << std::endl;

                EXPECT(commutative);

                if (A.empty() || B.empty()) {
                    EXPECT(AiB.empty());
                    EXPECT(BiA.empty());
                } else {
                    EXPECT(AiB == BiA);
                    EXPECT(A.contains(AiB));
                    EXPECT(B.contains(AiB));
                }

                if (A.isPeriodicWestEast() && B.isPeriodicWestEast()) {
                    EXPECT(AiB.isPeriodicWestEast());
                }

                if (!AiB.empty()) {
                    if (A.isPeriodicWestEast() != B.isPeriodicWestEast()) {
                        BoundingBox Awe = { AiB.north(), A.west(), AiB.south(), A.east() };
                        BoundingBox Bwe = { AiB.north(), B.west(), AiB.south(), B.east() };
                        EXPECT(AiB == Awe || AiB == Bwe);
                    }

                    const std::vector<Longitude> lims {
                        { A.west() },
                        { A.east() },
                        { B.west() },
                        { B.east() },
                    };
                    auto n = std::count_if(lims.begin(), lims.end(), [&AiB](const Longitude& lon) {
                        return  AiB.contains(AiB.north(), lon) ||
                                AiB.contains(AiB.south(), lon);
                    });
                    EXPECT(n = 2 || n == 4);
                }
            }
        }
    }

    log.precision(old);
}


CASE("Representation::extendedBoundingBox") {

    using namedgrids::NamedGrid;
    using util::BoundingBox;

    auto& log = eckit::Log::info();
    auto old = log.precision(16);

    SECTION("Gaussian") {

        for (const auto& name : {
             "F16",
             "O16",
             "N16",
             "F21",
             "O21",
            }) {
            repres::RepresentationHandle repres = NamedGrid::lookup(name).representation();

            for (const auto& bbox : {
                     BoundingBox(90, 10, -10, 9),
                     BoundingBox(90, -350, -10, 9),
                     BoundingBox(-70, -10, -90, 10),
                     BoundingBox(0, -350, 0, 9),
                     BoundingBox(60, -350, 20, 9),
                     BoundingBox(90, -10, 70, 10),
                 }) {
                BoundingBox extended = repres->extendedBoundingBox(bbox);

                log << name << "\t > " << *repres << " + extendedBoundingBox("
                    << "\n\t   " << bbox << ")"
                    << "\n\t = " << extended
                    << std::endl;

                EXPECT(extended.contains(bbox));
            }
        }
    }

    log.precision(old);
}


} // namespace unit
} // namespace tests
} // namespace mir


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv, false);
}

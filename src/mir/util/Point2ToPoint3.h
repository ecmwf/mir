// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "mir/util/Types.h"


namespace mir::repres {
class Representation;
}


namespace mir::util {


struct Point2ToPoint3 final {
    Point2ToPoint3(const repres::Representation&, double poleDisplacement);
    Point3 operator()(const Point2&) const;

private:
    struct Calculate {
        Calculate()          = default;
        virtual ~Calculate() = default;

        virtual Point3 make_point3(const Point2&) const = 0;

        Calculate(const Calculate&) = delete;
        Calculate(Calculate&&)      = delete;

        void operator=(const Calculate&) = delete;
        void operator=(Calculate&&)      = delete;
    };

    struct Point3Simple final : Calculate {
        Point3 make_point3(const Point2&) const override;
    };

    struct Point3Displaced final : Calculate {
        explicit Point3Displaced(double poleDisplacement);
        Point3 make_point3(const Point2&) const override;
        const double eps_;
    };

    std::unique_ptr<Calculate> calculate_;
};


}  // namespace mir::util

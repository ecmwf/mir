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

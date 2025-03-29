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


struct Point2ToPointXYZ final {
    Point2ToPointXYZ(const repres::Representation&, double poleDisplacement);
    PointXYZ operator()(const PointXY&) const;

private:
    struct Calculate {
        Calculate()          = default;
        virtual ~Calculate() = default;

        virtual PointXYZ make_point3(const PointXY&) const = 0;

        Calculate(const Calculate&) = delete;
        Calculate(Calculate&&)      = delete;

        void operator=(const Calculate&) = delete;
        void operator=(Calculate&&)      = delete;
    };

    struct PointXYZSimple final : Calculate {
        PointXYZ make_point3(const PointXY&) const override;
    };

    struct PointXYZDisplaced final : Calculate {
        explicit PointXYZDisplaced(double poleDisplacement);
        PointXYZ make_point3(const PointXY&) const override;
        const double eps_;
    };

    std::unique_ptr<Calculate> calculate_;
};


}  // namespace mir::util

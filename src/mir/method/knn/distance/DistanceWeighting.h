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

#include <iosfwd>

#include "mir/method/WeightMatrix.h"
#include "mir/search/PointSearch.h"


namespace eckit {
class MD5;
}


namespace mir {
namespace method {
namespace knn {
namespace distance {


class DistanceWeighting {
public:
    DistanceWeighting();

    virtual ~DistanceWeighting();

    virtual void operator()(size_t ip, const Point3& point,
                            const std::vector<search::PointSearch::PointValueType>& neighbours,
                            std::vector<WeightMatrix::Triplet>& triplets) const = 0;

    virtual bool sameAs(const DistanceWeighting&) const = 0;

    virtual void hash(eckit::MD5&) const = 0;

private:
    DistanceWeighting(const DistanceWeighting&) = delete;
    DistanceWeighting& operator=(const DistanceWeighting&) = delete;

    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const DistanceWeighting& p) {
        p.print(s);
        return s;
    }
};


class DistanceWeightingFactory {
private:
    std::string name_;
    virtual DistanceWeighting* make(const param::MIRParametrisation&) = 0;

    DistanceWeightingFactory(const DistanceWeightingFactory&) = delete;
    DistanceWeightingFactory& operator=(const DistanceWeightingFactory&) = delete;

protected:
    DistanceWeightingFactory(const std::string& name);
    virtual ~DistanceWeightingFactory();

public:
    static const DistanceWeighting* build(const std::string& name, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class DistanceWeightingBuilder : public DistanceWeightingFactory {
    DistanceWeighting* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    DistanceWeightingBuilder(const std::string& name) : DistanceWeightingFactory(name) {}
};


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

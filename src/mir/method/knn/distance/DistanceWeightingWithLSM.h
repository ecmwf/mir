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

#include "mir/method/knn/distance/DistanceWeighting.h"

#include "mir/util/Exceptions.h"


namespace mir {
namespace lsm {
class LandSeaMasks;
}
}  // namespace mir


namespace mir {
namespace method {
namespace knn {
namespace distance {


struct DistanceWeightingWithLSM : DistanceWeighting {

    DistanceWeightingWithLSM(const param::MIRParametrisation&);

    void operator()(size_t, const Point3&, const std::vector<search::PointSearch::PointValueType>&,
                    std::vector<WeightMatrix::Triplet>&) const override {
        throw exception::SeriousBug("DistanceWeightingWithLSM: not to be used directly");
    }

    const DistanceWeighting* distanceWeighting(const param::MIRParametrisation&, const lsm::LandSeaMasks& lsm) const;

private:
    std::string method_;
    bool sameAs(const DistanceWeighting&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
};


class DistanceWeightingWithLSMFactory {
private:
    std::string name_;
    virtual DistanceWeighting* make(const param::MIRParametrisation&, const lsm::LandSeaMasks&) = 0;

    DistanceWeightingWithLSMFactory(const DistanceWeightingWithLSMFactory&) = delete;
    DistanceWeightingWithLSMFactory& operator=(const DistanceWeightingWithLSMFactory&) = delete;

protected:
    DistanceWeightingWithLSMFactory(const std::string& name);
    virtual ~DistanceWeightingWithLSMFactory();

public:
    static const DistanceWeighting* build(const std::string& name, const param::MIRParametrisation&,
                                          const lsm::LandSeaMasks&);
    static void list(std::ostream&);
    static bool has(const std::string& name);
};


template <class T>
class DistanceWeightingWithLSMBuilder : public DistanceWeightingWithLSMFactory {
    DistanceWeighting* make(const param::MIRParametrisation& param, const lsm::LandSeaMasks& lsm) override {
        return new T(param, lsm);
    }

public:
    DistanceWeightingWithLSMBuilder(const std::string& name) : DistanceWeightingWithLSMFactory(name) {}
};


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

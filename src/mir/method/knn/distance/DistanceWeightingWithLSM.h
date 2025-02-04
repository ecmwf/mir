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


namespace mir::lsm {
class LandSeaMasks;
}  // namespace mir::lsm


namespace mir::method::knn::distance {


class DistanceWeightingWithLSM : public DistanceWeighting {
public:
    explicit DistanceWeightingWithLSM(const param::MIRParametrisation&);

    void operator()(size_t, const Point3&, const std::vector<search::PointSearch::PointValueType>&,
                    std::vector<WeightMatrix::Triplet>&) const override;

    const DistanceWeighting* distanceWeighting(const param::MIRParametrisation&, const lsm::LandSeaMasks&) const;

private:
    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    std::string method_;
};


class DistanceWeightingWithLSMFactory {
private:
    std::string name_;
    virtual DistanceWeighting* make(const param::MIRParametrisation&, const lsm::LandSeaMasks&) = 0;

protected:
    explicit DistanceWeightingWithLSMFactory(const std::string& name);
    virtual ~DistanceWeightingWithLSMFactory();

public:
    DistanceWeightingWithLSMFactory(const DistanceWeightingWithLSMFactory&) = delete;
    DistanceWeightingWithLSMFactory(DistanceWeightingWithLSMFactory&&)      = delete;

    void operator=(const DistanceWeightingWithLSMFactory&) = delete;
    void operator=(DistanceWeightingWithLSMFactory&&)      = delete;

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
    explicit DistanceWeightingWithLSMBuilder(const std::string& name) : DistanceWeightingWithLSMFactory(name) {}
};


}  // namespace mir::method::knn::distance

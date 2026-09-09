// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/method/WeightMatrix.h"
#include "mir/search/PointSearch.h"


namespace eckit {
class JSON;
class MD5;
}  // namespace eckit


namespace mir::method::knn::distance {


class DistanceWeighting {
public:
    DistanceWeighting();

    DistanceWeighting(const DistanceWeighting&) = delete;
    DistanceWeighting(DistanceWeighting&&)      = delete;

    DistanceWeighting& operator=(const DistanceWeighting&) = delete;
    DistanceWeighting& operator=(DistanceWeighting&&)      = delete;

    virtual ~DistanceWeighting();

    virtual void operator()(size_t ip, const Point3& point,
                            const std::vector<search::PointSearch::PointValueType>& neighbours,
                            std::vector<WeightMatrix::Triplet>& triplets) const = 0;

    virtual bool sameAs(const DistanceWeighting&) const = 0;

    virtual void hash(eckit::MD5&) const = 0;

    virtual WeightMatrix::Check validateMatrixWeights() const;

    virtual void json(eckit::JSON&) const = 0;

protected:
    virtual const std::string& type() const;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const DistanceWeighting& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const DistanceWeighting& p) {
        p.json(s);
        return s;
    }
};


class DistanceWeightingFactory {
private:
    std::string name_;
    virtual DistanceWeighting* make(const param::MIRParametrisation&) = 0;

protected:
    explicit DistanceWeightingFactory(const std::string& name);
    virtual ~DistanceWeightingFactory();

public:
    DistanceWeightingFactory(const DistanceWeightingFactory&) = delete;
    DistanceWeightingFactory(DistanceWeightingFactory&&)      = delete;

    DistanceWeightingFactory& operator=(const DistanceWeightingFactory&) = delete;
    DistanceWeightingFactory& operator=(DistanceWeightingFactory&&)      = delete;

    static const DistanceWeighting* build(const std::string& name, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class DistanceWeightingBuilder : public DistanceWeightingFactory {
    DistanceWeighting* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    explicit DistanceWeightingBuilder(const std::string& name) : DistanceWeightingFactory(name) {}
};


}  // namespace mir::method::knn::distance

// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/pick/SortedSample.h"

#include <algorithm>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"
#include "mir/param/DefaultParametrisation.h"


namespace mir::method::knn::pick {


SortedSample::SortedSample(const param::MIRParametrisation& param) : sample_(param) {}


void SortedSample::pick(const search::PointSearch& tree, const Point3& p, Pick::neighbours_t& closest) const {
    sample_.pick(tree, p, closest);

    std::sort(closest.begin(), closest.end(),
              [&p](const Pick::neighbours_t::value_type& a, const Pick::neighbours_t::value_type& b) {
                  return Point3::distance2(a.point(), p) < Point3::distance2(b.point(), p);
              });
}

size_t SortedSample::n() const {
    return sample_.n();
}


bool SortedSample::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const SortedSample*>(&other);
    return (o != nullptr) && sample_.sameAs(*o);
}


void SortedSample::hash(eckit::MD5& h) const {
    h << "sample-sorted";
    h << sample_;
}


void SortedSample::json(eckit::JSON& j) const {
    j << type() << "sorted-sample";
    param::DefaultParametrisation::instance().json(j, "nclosest", sample_.n());
    param::DefaultParametrisation::instance().json(j, "distance", sample_.d());
}


void SortedSample::print(std::ostream& out) const {
    out << "SortedSample[sample=" << sample_ << "]";
}


static const PickBuilder<SortedSample> __pick("sorted-sample");


}  // namespace mir::method::knn::pick

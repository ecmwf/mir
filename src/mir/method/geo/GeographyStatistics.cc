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


#include "mir/method/geo/GeographyStatistics.h"

#include "eckit/utils/MD5.h"

#include "mir/method/geo/GeographyToRepresentation.h"
#include "mir/method/geo/RepresentationToGeography.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace method {
namespace geo {


GeographyStatistics::GeographyStatistics(const param::MIRParametrisation& param) :
    Method(param),
    r2g_(new RepresentationToGeography(param)),
    g2r_(new GeographyToRepresentation(param)),
    geography_(param) {
    ASSERT(r2g_);
    ASSERT(g2r_);
}


void GeographyStatistics::hash(eckit::MD5& h) const {
    h << version();
    h << *r2g_;
    h << *g2r_;
    h << geography_;
}


int GeographyStatistics::version() const {
    return 0;
}


void GeographyStatistics::execute(context::Context& ctx, const repres::Representation& in,
                                  const repres::Representation& out) const {
    r2g_->execute(ctx, in, geography_);
    g2r_->execute(ctx, geography_, out);
}


bool GeographyStatistics::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const GeographyStatistics*>(&other);
    return (o != nullptr) && static_cast<const repres::Representation&>(geography_).sameAs(o->geography_) &&
           r2g_->sameAs(*o->r2g_) && g2r_->sameAs(*o->g2r_);
}


bool GeographyStatistics::canCrop() const {
    return false;
}


void GeographyStatistics::setCropping(const util::BoundingBox& bbox) {
    NOTIMP;
}


bool GeographyStatistics::hasCropping() const {
    return false;
}


const util::BoundingBox& GeographyStatistics::getCropping() const {
    NOTIMP;
}


void GeographyStatistics::print(std::ostream& out) const {
    out << "GeographyStatistics[representationToGeography=" << *r2g_ << ",geographyToRepresentation=" << *g2r_
        << ",geography=" << geography_ << "]";
}


static const MethodBuilder<GeographyStatistics> __builder("geography-statistics");


}  // namespace geo
}  // namespace method
}  // namespace mir

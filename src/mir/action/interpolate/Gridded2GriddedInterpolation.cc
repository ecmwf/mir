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


#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

#include <sstream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/method/Cropping.h"
#include "mir/method/Method.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2GriddedInterpolation::Gridded2GriddedInterpolation(const param::MIRParametrisation& param) : Action(param) {

    ASSERT(param.get("interpolation", interpolation_));
    method_.reset(method::MethodFactory::build(interpolation_, param));
    ASSERT(method_);

    inputIntersectsOutput_ = !param.has("rotation");
}


Gridded2GriddedInterpolation::~Gridded2GriddedInterpolation() = default;


const method::Method& Gridded2GriddedInterpolation::method() const {
    ASSERT(method_);
    return *method_;
}


bool Gridded2GriddedInterpolation::mergeWithNext(const Action& next) {
    if (next.canCrop() && method_->canCrop()) {
        method_->setCropping(next.outputBoundingBox());
        return true;
    }
    return false;
}


bool Gridded2GriddedInterpolation::canCrop() const {
    return method_->hasCropping();
}


method::Cropping Gridded2GriddedInterpolation::cropping(context::Context& ctx) const {

    const data::MIRField& field = ctx.field();

    repres::RepresentationHandle in(field.representation());
    auto input = in->domain();
    auto output(outputBoundingBox());

    method::Cropping crop;
    if (method_->hasCropping()) {
        crop.boundingBox(method_->getCropping());
    }

    if (!input.isGlobal()) {
        if (inputIntersectsOutput_) {

            repres::RepresentationHandle out(outputRepresentation());
            if (out->extendBoundingBoxOnIntersect()) {
                out->extendBoundingBox(input).intersects(output);
            }
            else {
                input.intersects(output);
            }

            if (crop) {
                crop.boundingBox().intersects(output);
            }
            crop.boundingBox(output);
        }
        else if (!input.contains(output)) {
            std::ostringstream msg;
            msg << "Input does not contain output:"
                << "\n\t"
                   "Input: "
                << input
                << "\n\t"
                   "Output: "
                << outputBoundingBox();
            throw exception::UserError(msg.str());
        }
    }

    if (crop) {
        // disable cropping if global and West aligned with input (no "renumbering")
        auto& bbox = crop.boundingBox();
        const util::Domain domain(bbox.north(), bbox.west(), bbox.south(), bbox.east());
        if (domain.isGlobal() && bbox.west() == input.west()) {
            return method::Cropping();
        }
    }

    return crop;
}


void Gridded2GriddedInterpolation::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().grid2gridTimer());

    auto& field = ctx.field();
    repres::RepresentationHandle in(field.representation());

    method::Cropping crop = cropping(ctx);

    repres::RepresentationHandle output(outputRepresentation());
    repres::RepresentationHandle out(crop ? output->croppedRepresentation(crop.boundingBox()) : output.operator->());

    method_->execute(ctx, *in, *out);

    field.representation(out);
}


bool Gridded2GriddedInterpolation::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2GriddedInterpolation*>(&other);
    return (o != nullptr) && (interpolation_ == o->interpolation_) && method_->sameAs(*o->method_) &&
           (inputIntersectsOutput_ == o->inputIntersectsOutput_);
}


void Gridded2GriddedInterpolation::print(std::ostream& out) const {
    out << "interpolation=" << interpolation_ << ",method=" << *method_;
}

void Gridded2GriddedInterpolation::estimate(context::Context& ctx, api::MIREstimation& estimation) const {

    method::Cropping crop = cropping(ctx);

    repres::RepresentationHandle output(outputRepresentation());
    repres::RepresentationHandle out(crop ? output->croppedRepresentation(crop.boundingBox()) : output.operator->());

    estimateNumberOfGridPoints(ctx, estimation, *out);
    estimateMissingValues(ctx, estimation, *out);

    ctx.field().representation(out);
}


}  // namespace interpolate
}  // namespace action
}  // namespace mir

// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

#include <sstream>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/value/Value.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/method/Cropping.h"
#include "mir/method/Method.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Intersect.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action::interpolate {


Gridded2GriddedInterpolation::Gridded2GriddedInterpolation(const param::MIRParametrisation& param) : Action(param) {
    ASSERT(param.get("interpolation", interpolation_));
    method_.reset(method::MethodFactory::build(interpolation_, param));
    ASSERT(method_);

    param.get("interpolation-global-input", inputGlobal_ = false);
    param.get("interpolation-global-output", outputGlobal_ = false);

    // as by plan compression (see mergeWithNext)
    if (std::vector<double> crop; param.get("cropping", crop)) {
        ASSERT(crop.size() == 4);
        method_->setCropping({crop[0], crop[1], crop[2], crop[3]});
    }
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
    const auto& field = ctx.field();

    repres::RepresentationHandle in(field.representation());

    method::Cropping crop;
    if (method_->hasCropping()) {
        crop.boundingBox(method_->getCropping());
    }

    auto in_bbox = inputGlobal_ ? util::Domain{} : in->geographicDomain();
    if (!in_bbox.isGlobal()) {
        repres::RepresentationHandle out(outputRepresentation());
        auto out_bbox = outputGlobal_ ? util::Domain{} : outputBoundingBox();

        util::Intersect::build(out->intersectionOnCrop()).apply(*in, in_bbox, *out, out_bbox, crop);
    }

    if (crop) {
        // disable cropping if global and West aligned with input (no "renumbering")
        const auto& bbox = crop.boundingBox();
        const util::Domain domain(bbox.north(), bbox.west(), bbox.south(), bbox.east());
        if (domain.isGlobal() && bbox.west() == in_bbox.west()) {
            return {};
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
    const auto* o = dynamic_cast<const Gridded2GriddedInterpolation*>(&other);
    return (o != nullptr) && (interpolation_ == o->interpolation_) && method_->sameAs(*o->method_);
}


void Gridded2GriddedInterpolation::json(eckit::JSON& j) const {
    // the method spec (complete), flattened (as interpolation options)
    j << "interpolation" << interpolation_;

    const auto str = [this]() {
        param::DefaultParametrisation::JSONComplete complete;
        return method_->json_str(false);
    }();

    if (const auto spec = eckit::YAMLParser::decodeString(str); spec.isMap()) {
        for (const auto& kv : eckit::ValueMap(spec)) {
            if (const auto k = kv.first.as<std::string>(); k != "type") {
                j << k << kv.second;
            }
        }
    }

    if (method_->hasCropping()) {
        j << "cropping" << spec_value(method_->getCropping());
    }

    if (inputGlobal_) {
        j << "interpolation-global-input" << inputGlobal_;
    }

    if (outputGlobal_) {
        j << "interpolation-global-output" << outputGlobal_;
    }
}


void Gridded2GriddedInterpolation::print(std::ostream& out) const {
    out << "interpolation=" << interpolation_ << ",method=" << *method_;
}


}  // namespace mir::action::interpolate

/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Apr 2015


#include "mir/action/transform/ShToGridded.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/geometry/UnitSphere.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/log/Timer.h"
#include "eckit/system/SystemInfo.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/action/transform/TransCache.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/caching/LegendreCache.h"
#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/function/FunctionParser.h"


namespace mir {
namespace action {
namespace transform {


static eckit::Mutex amutex;


static InMemoryCache<TransCache> trans_cache("mirCoefficient",
        8L * 1024 * 1024 * 1024,
        8L * 1024 * 1024 * 1024,
        "$MIR_COEFFICIENT_CACHE",
        false); // Don't cleanup at exit: the Fortran part will dump core


static ShToGridded::atlas_trans_t getTrans(
        const param::MIRParametrisation& parametrisation,
        context::Context& ctx,
        const std::string& key,
        const atlas::Grid& grid,
        size_t truncation,
        const ShToGridded::atlas_config_t& options) {


    InMemoryCache<TransCache>::iterator j = trans_cache.find(key);
    if (j != trans_cache.end()) {
        return j->trans_;
    }


    if (!parametrisation.has("caching")) {
        TransCache& tc = trans_cache[key];
        ShToGridded::atlas_trans_t& trans = tc.trans_;

        trans = ShToGridded::atlas_trans_t(grid, int(truncation), options);
        ASSERT(trans);
        return trans;
    }

    // Make sure we have enough space in cache to add new coefficients
    // otherwise we may get killed by OOM thread
    size_t estimate = truncation * truncation * truncation / 2 * sizeof(double);
    trans_cache.reserve(estimate, caching::legendre::LegendreLoaderFactory::inSharedMemory(parametrisation));

    eckit::PathName path;

    {   // Block for timers

        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().coefficientTiming_);

        class LegendreCacheCreator final : public caching::LegendreCache::CacheContentCreator {

            context::Context& ctx_;
            const atlas::Grid& grid_;
            const size_t truncation_;
            const ShToGridded::atlas_config_t& options_;

            void create(const eckit::PathName& path, caching::LegendreCacheTraits::value_type& /*ignore*/, bool& saved) override {
                eckit::TraceResourceUsage<LibMir> usage("ShToGridded: create coefficients");
                eckit::AutoTiming timing(ctx_.statistics().timer_, ctx_.statistics().createCoeffTiming_);

                ShToGridded::atlas_config_t write(options_);
                write.set(atlas::option::write_legendre(path));

                // This will create the cache
                ShToGridded::atlas_trans_t tmp(grid_, int(truncation_), write);
                ASSERT(tmp);

                saved = path.exists();
            }
        public:
            LegendreCacheCreator(
                        context::Context& ctx,
                        const atlas::Grid& grid,
                        size_t truncation,
                        const ShToGridded::atlas_config_t& options) :
                ctx_(ctx),
                grid_(grid),
                truncation_(truncation),
                options_(options) {
                ASSERT(!options_.has("read_legendre"));
            }
        };

        static caching::LegendreCache cache;
        LegendreCacheCreator creator(ctx, grid, truncation, options);

        int dummy = 0;
        path = cache.getOrCreate(key, creator, dummy);
    }


    TransCache& tc = trans_cache[key];
    ShToGridded::atlas_trans_t& trans = tc.trans_;

    {
        eckit::TraceResourceUsage<LibMir> usage("ShToGridded: load coefficients");
        eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().loadCoeffTiming_);

        const eckit::system::MemoryInfo before = eckit::system::SystemInfo::instance().memoryUsage();

        tc.inited_ = true;
        tc.loader_ = caching::legendre::LegendreLoaderFactory::build(parametrisation, path);
        ASSERT(tc.loader_);
        eckit::Log::debug<LibMir>() << "ShToGridded: LegendreLoader " << *tc.loader_ << std::endl;

        trans = ShToGridded::atlas_trans_t(tc.loader_->transCache(), grid, int(truncation), options);

        eckit::system::MemoryInfo after = eckit::system::SystemInfo::instance().memoryUsage();
        after.delta(eckit::Log::info(), before);

        size_t memory = 0;
        size_t shared = 0;
        (tc.loader_->inSharedMemory() ? shared : memory) = tc.loader_->size();
        trans_cache.footprint(key, InMemoryCacheUsage(memory, shared));
    }

    ASSERT(trans);
    return trans;
}


void ShToGridded::transform(data::MIRField& field, const repres::Representation& representation, context::Context& ctx) const {
    eckit::AutoLock<eckit::Mutex> lock(amutex); // To protect trans_cache

    // Make sure another thread to no evict anything from the cache while we are using it
    // FIXME check if it should be in ::execute()
    InMemoryCacheUser<TransCache> use(trans_cache, ctx.statistics().transHandleCache_);


    const size_t truncation = field.representation()->truncation();
    ASSERT(truncation);

    const std::string key =
            "T" + std::to_string(truncation)
            + ":" + representation.uniqueName()
            + ":" + options_.digest();

    // set grid and options (options include global grid, if cropping)
    atlas::Grid grid = representation.atlasGrid();
    ASSERT(grid);

    atlas_config_t options(options_);
    if (cropping_) {
        ASSERT(local());
        const util::BoundingBox& bbox = cropping_.boundingBox();
        repres::RepresentationHandle local(representation.croppedRepresentation(bbox));

        options = options | atlas::option::global_grid(grid);
        grid = local->atlasGrid();
    }

    atlas_trans_t trans;
    try {

        eckit::Timer time("ShToGridded::transform: setup", eckit::Log::debug<LibMir>());
        trans = getTrans(parametrisation_,
                         ctx,
                         key,
                         grid,
                         truncation,
                         options);

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::transform: setup: " << e.what() << std::endl;
        trans_cache.erase(key);
        throw;
    }
    ASSERT(trans);

    try {

        eckit::AutoTiming time(ctx.statistics().timer_, ctx.statistics().sh2gridTiming_);
        sh2grid(field, trans, grid);

    } catch (std::exception& e) {
        eckit::Log::error() << "ShToGridded::transform: invtrans: " << e.what() << std::endl;
        throw;
    }
}


ShToGridded::ShToGridded(const param::MIRParametrisation& parametrisation) :
    Action(parametrisation) {
    const param::MIRParametrisation& user = parametrisation.userParametrisation();

    // set compression condition: default is don't compress (unless strictly necessary)
    std::string compressIf = "0";
    user.get("transform-compress-if", compressIf);
    std::istringstream in(compressIf);
    util::function::FunctionParser p(in);

    compressIf_.reset(p.parse());
    ASSERT(compressIf_);

    if (user.has("atlas-trans-local")) {
        local(true);
    }

    // TODO: MIR-183 let Trans decide the best Legendre transform method
    bool flt = false;
    user.get("atlas-trans-flt", flt);
    options_.set("flt", flt);

    // no partitioning
    options_.set(atlas::option::global());
}


ShToGridded::~ShToGridded() {
}


void ShToGridded::print(std::ostream& out) const {
    out <<  "cropping=" << cropping_
        << ",local=" << local()
        << ",options=[" << options_.digest() << "]";
}


void ShToGridded::execute(context::Context& ctx) const {

    repres::RepresentationHandle out(outputRepresentation());

    transform(ctx.field(), *out, ctx);

    if (cropping_) {
        repres::RepresentationHandle local(out->croppedRepresentation(cropping_.boundingBox()));

        ctx.field().representation(local);
        ctx.field().validate();

    } else {

        ctx.field().representation(out);

    }
}


bool ShToGridded::mergeWithNext(const Action& next) {

    // make use of the area cropping action downstream (no merge)
    if (!cropping_ && next.canCrop()) {
        const util::BoundingBox& bbox = next.croppingBoundingBox();

        if (!local()) {

            // evaluate according to bounding box and area ratio to globe
            eckit::geometry::Point2 WestNorth(bbox.west().value(), bbox.north().value());
            eckit::geometry::Point2 EastSouth(bbox.east().value(), bbox.south().value());

            double ar = atlas::util::Earth::area(WestNorth, EastSouth)
                      / atlas::util::Earth::area();

            param::SimpleParametrisation vars;
            vars.set("N", WestNorth[1]);
            vars.set("W", WestNorth[0]);
            vars.set("S", EastSouth[1]);
            vars.set("E", EastSouth[0]);
            vars.set("ar", ar);

            if (!bool(compressIf_->eval(vars))) {
                return false;
            }
        }

        repres::RepresentationHandle out(outputRepresentation());

        // if directly followed by cropping go straight to the cropped representation
        if (next.isCropAction()) {
            cropping_.boundingBox(out->croppedBoundingBox(bbox));
            local(true);
            return true;
        }

        // extend bbox with element diagonals [m], converted to (central) angle
        double radius = 0;
        ASSERT(out->getLongestElementDiagonal(radius));
        ASSERT(radius > 0);

        double c = radius / atlas::util::Earth::radius();
        double angle = util::radian_to_degree(2. * std::asin(0.5 * c));

        // Extend the box, then crop to produce a valid representation
        util::BoundingBox extended = next.extendedBoundingBox(bbox, angle);
        util::BoundingBox best = out->croppedBoundingBox(extended);

        eckit::Log::debug<LibMir>()
                << "ShToGridded::mergeWithNext: "
                << "\n\t   " << *this
                << "\n\t + " << next
                << std::endl;

        // Magic super-powers!
        cropping_.boundingBox(best);
        local(true);

    }
    return false;
}


void ShToGridded::local(bool l) {
    if (!l) {
        if (local()) {
            throw eckit::SeriousBug("ShToGridded::local: Atlas/Trans 'local' has been set, cannot revert");
        }
        return;
    }
    options_.set(atlas::option::type("local"));
}


bool ShToGridded::local() const {
    return options_.has("type") && options_.getString("type") == "local";
}


bool ShToGridded::sameAs(const Action& other) const {
    const ShToGridded* o = dynamic_cast<const ShToGridded*>(&other);
    return o && options_.digest() == o->options_.digest();
}


eckit::Hash::digest_t ShToGridded::atlas_config_t::digest() const {
    // We don't want to 'see' the internal options, just if they are set differently
    // (so we know when they change)
    eckit::MD5 h;
    this->hash(h);
    return h.digest();
}


}  // namespace transform
}  // namespace action
}  // namespace mir

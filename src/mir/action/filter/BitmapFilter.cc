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


#include "mir/action/filter/BitmapFilter.h"

#include <ostream>
#include <sstream>

#include "mir/action/context/Context.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Bitmap.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/Mutex.h"


namespace mir::action {


constexpr size_t CAPACITY = 256 * 1024 * 1024;
static caching::InMemoryCache<util::Bitmap> cache("mirBitmap", CAPACITY, 0, "$MIR_BITMAP_CACHE_MEMORY_FOOTPRINT");


BitmapFilter::BitmapFilter(const param::MIRParametrisation& parametrisation) : Action(parametrisation) {
    ASSERT(parametrisation.userParametrisation().get("bitmap", path_));
}


BitmapFilter::~BitmapFilter() = default;


bool BitmapFilter::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const BitmapFilter*>(&other);
    return (o != nullptr) && (path_ == o->path_);
}


void BitmapFilter::print(std::ostream& out) const {
    out << "BitmapFilter[path=" << path_ << "]";
}


util::Bitmap& BitmapFilter::bitmap() const {
    static util::recursive_mutex local_mutex;
    util::lock_guard<util::recursive_mutex> lock(local_mutex);

    auto* j = cache.find(path_);
    if (j != cache.end()) {
        return *j;
    }

    auto& bitmap = cache.insert(path_, new util::Bitmap(path_));
    cache.footprint(path_, caching::InMemoryCacheUsage(bitmap.footprint(), 0));
    return bitmap;
}


void BitmapFilter::execute(context::Context& ctx) const {

    // Make sure another thread to no evict anything from the cache while we are using it
    auto cacheUse(ctx.statistics().cacheUser(cache));
    auto timing(ctx.statistics().bitmapTimer());

    auto& field = ctx.field();
    auto& b     = bitmap();

    for (size_t f = 0; f < field.dimensions(); f++) {

        auto missingValue = field.missingValue();
        auto& values      = field.direct(f);

        // if (values.size() != b.width() * b.height()) {
        if (values.size() > b.width() * b.height()) {  // TODO: fixe me
            std::ostringstream os;
            os << "BitmapFilter::execute size mismatch: values=" << values.size() << ", bitmap=" << b.width() << "x"
               << b.height() << "=" << b.width() * b.height();

            throw exception::UserError(os.str());
        }


        size_t k = 0;
        for (size_t j = 0; j < b.height(); j++) {
            for (size_t i = 0; i < b.width(); ++i, ++k) {

                if (k == values.size()) {
                    // Temp fix
                    break;
                }

                if (!b.on(j, i)) {
                    values[k] = missingValue;
                }
            }
        }

        field.hasMissing(true);
    }
}


const char* BitmapFilter::name() const {
    return "BitmapFilter";
}


static const ActionBuilder<BitmapFilter> __action("filter.bitmap");


}  // namespace mir::action

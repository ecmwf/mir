/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <iostream>

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/action/context/Context.h"
#include "mir/util/Bitmap.h"

#include "mir/action/filter/BitmapFilter.h"
#include "mir/util/MIRStatistics.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/data/MIRField.h"


namespace mir {
namespace action {


namespace {
static eckit::Mutex local_mutex;
static InMemoryCache<util::Bitmap> cache("mirBitmap", 256 * 1024 * 1024, "$MIR_BITMAP_CACHE_MEMORY_FOOTPRINT", true);
}


BitmapFilter::BitmapFilter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
    ASSERT(parametrisation.get("user.bitmap", path_));
}


BitmapFilter::~BitmapFilter() {
}




bool BitmapFilter::sameAs(const Action& other) const {
    const BitmapFilter* o = dynamic_cast<const BitmapFilter*>(&other);
    return o && (path_ == o->path_);
}

void BitmapFilter::print(std::ostream &out) const {
    out << "BitmapFilter[path=" << path_ << "]";
}


util::Bitmap& BitmapFilter::bitmap() const {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    InMemoryCache<util::Bitmap>::iterator j  = cache.find(path_);
    if (j == cache.end()) {
        eckit::ScopedPtr<util::Bitmap> bitmap(new util::Bitmap(path_));
        size_t footprint = bitmap->footprint();
        util::Bitmap& result = cache.insert(path_, bitmap.release());
        cache.footprint(path_, footprint);
        return result;
    }
    return *j;
}

void BitmapFilter::execute(context::Context & ctx) const {

    // Make sure another thread to no evict anything from the cache while we are using it
    InMemoryCacheUser<util::Bitmap> use(cache, ctx.statistics().bitmapCache_);

    eckit::AutoTiming timing(ctx.statistics().timer_, ctx.statistics().bitmapTiming_);
    data::MIRField& field = ctx.field();

    util::Bitmap& b = bitmap();

    for (size_t f = 0; f < field.dimensions() ; f++) {

        double missingValue = field.missingValue();
        std::vector<double> &values = field.direct(f);

        // if (values.size() != b.width() * b.height()) {
        if (values.size() > b.width() * b.height()) { // TODO: fixe me
            std::ostringstream os;
            os << "BitmapFilter::execute size mismatch: values="
               << values.size()
               << ", bitmap=" << b.width() << "x" << b.height()
               << "="
               <<  b.width() * b.height();

            throw eckit::UserError(os.str());
        }


        size_t k = 0;
        for (size_t j = 0; j < b.height() ; j++ ) {

            for (size_t i = 0; i < b.width() ; i++ ) {

                if (k == values.size()) {
                    // Temp fix
                    break;
                }

                if (!b.on(j, i)) {
                    values[k] = missingValue;
                }
                k++;


            }
        }

        field.hasMissing(true);
    }
}


namespace {
static ActionBuilder< BitmapFilter > bitmapFilter("filter.bitmap");
}


}  // namespace action
}  // namespace mir


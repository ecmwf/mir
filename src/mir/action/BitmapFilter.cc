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
#include "mir/data/MIRField.h"
#include "mir/util/Bitmap.h"

#include "mir/action/BitmapFilter.h"


namespace mir {
namespace action {


namespace {
static eckit::Mutex local_mutex;
static std::map< std::string, util::Bitmap *> cache;
}


BitmapFilter::BitmapFilter(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {

    std::string path;
    ASSERT(parametrisation.get("user.bitmap", path));

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, util::Bitmap *>::iterator j = cache.find(path);
    if (j == cache.end()) {
        bitmap_ = cache[path] = new util::Bitmap(path);
    } else {
        bitmap_ = (*j).second;
    }

}


BitmapFilter::~BitmapFilter() {
}


void BitmapFilter::print(std::ostream &out) const {
    out << "BitmapFilter[bitmap=" << *bitmap_ << "]";
}


void BitmapFilter::execute(data::MIRField &field) const {

    for (size_t i = 0; i < field.dimensions() ; i++) {

        double missingValue = field.missingValue();
        std::vector<double> &values = field.values(i);

        if (values.size() != bitmap_->width() * bitmap_->height()) {
            eckit::StrStream os;
            os << "BitmapFilter::execute size mismatch: values=" << values.size()
               << ", bitmap=" << bitmap_->width() << "x" << bitmap_->height()
               << eckit::StrStream::ends;

            throw eckit::UserError(std::string(os));
        }


        size_t k = 0;

        for (size_t h = 0; h < bitmap_->height() ; h++ ) {
            for (size_t w = 0; w < bitmap_->width() ; w++ ) {
                if (!bitmap_->on(w, h)) {
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


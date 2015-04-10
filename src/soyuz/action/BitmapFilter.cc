// File BitmapFilter.cc
// Baudouin Raoult - (c) ECMWF Apr 15

#include "soyuz/action/BitmapFilter.h"
#include "soyuz/param/MIRParametrisation.h"
#include "soyuz/util/MIRField.h"
#include "soyuz/util/Bitmap.h"

#include <iostream>


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

static eckit::Mutex local_mutex;
static std::map<std::string, Bitmap *> cache;


BitmapFilter::BitmapFilter(const MIRParametrisation &parametrisation):
    Action(parametrisation) {

    std::string path;
    ASSERT(parametrisation.get("bitmap", path));

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, Bitmap *>::iterator j = cache.find(path);
    if (j == cache.end()) {
        bitmap_ = cache[path] = new Bitmap(path);
    } else {
        bitmap_ = (*j).second;
    }

}

BitmapFilter::~BitmapFilter() {
}

void BitmapFilter::print(std::ostream &out) const {
    out << "BitmapFilter[bitmap=" << *bitmap_ << "]";
}

void BitmapFilter::execute(MIRField &field) const {

    double missingValue = field.missingValue();
    std::vector<double> &values = field.values();

    if (values.size() != bitmap_->width() * bitmap_->height()) {
        eckit::StrStream os;
        os << "BitmapFilter::execute size mismatch: values=" << values.size()
           << ", bitmap=" << bitmap_->width() << "x" << bitmap_->height()
           << eckit::StrStream::ends;

        throw eckit::UserError(std::string(os));
    }


    size_t k = 0;

    for (size_t j = 0; j < bitmap_->height() ; j++ ) {
        for (size_t i = 0; i < bitmap_->width() ; i++ ) {
            if (!bitmap_->on(i, j)) {
                values[k] = missingValue;
            }
            k++;
        }
    }

    field.hasMissing(true);

}

static ActionBuilder<BitmapFilter> bitmapFilter("filter.bitmap");

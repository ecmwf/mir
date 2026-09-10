// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Intersect.h"

#include <map>
#include <ostream>
#include <sstream>

#include "mir/method/Cropping.h"
#include "mir/repres/Representation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::util {


static recursive_mutex* local_mutex                = nullptr;
static std::map<std::string, IntersectFactory*>* m = nullptr;
static once_flag once;
static void init() {
    local_mutex = new recursive_mutex();
    m           = new std::map<std::string, IntersectFactory*>();
}


IntersectFactory::IntersectFactory(const std::string& name) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("Intersect: duplicate '" + name + "'");
    }

    (*m)[name] = this;
}


detail::Intersect* IntersectFactory::build(const std::string& name) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    Log::debug() << "Intersect: looking for '" << name << "'" << std::endl;
    if (auto j = m->find(name); j != m->end()) {
        return j->second->make();
    }

    list(Log::error() << "Intersect: unknown '" << name << "', choices are:\n");
    throw exception::SeriousBug("Intersect: unknown '" + name + "'");
}


void IntersectFactory::list(std::ostream& out) {
    call_once(once, init);
    lock_guard<recursive_mutex> lock(*local_mutex);

    const auto* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


Intersect::Intersect(detail::Intersect* ptr) : unique_ptr(ptr) {
    ASSERT(ptr);
}


struct InputIntersectsOutput final : detail::Intersect {
    void apply(const Representation& in, const BoundingBox& in_bbox, const Representation& out,
               const BoundingBox& out_bbox, Cropping& crop) const final {
        auto cropped = out_bbox;
        in_bbox.intersects(cropped);

        if (crop) {
            crop.boundingBox().intersects(cropped);
        }

        crop.boundingBox(cropped);
    }
};


struct InputExtendsOutput final : detail::Intersect {
    void apply(const Representation& in, const BoundingBox& in_bbox, const Representation& out,
               const BoundingBox& out_bbox, Cropping& crop) const final {
        auto cropped = out_bbox;
        out.extendBoundingBox(in_bbox).intersects(cropped);

        if (crop) {
            crop.boundingBox().intersects(cropped);
        }

        crop.boundingBox(cropped);
    }
};


struct InputContainsOutputCheck final : detail::Intersect {
    void apply(const Representation& in, const BoundingBox& in_bbox, const Representation& out,
               const BoundingBox& out_bbox, Cropping&) const final {
        if (!in_bbox.contains(out_bbox)) {
            std::ostringstream msg;
            msg << "Intersect: input does not contain output:"
                << "\n\t"
                   "Input: "
                << in_bbox
                << "\n\t"
                   "Output: "
                << out_bbox;
            throw exception::UserError(msg.str());
        }
    }
};


struct None final : detail::Intersect {
    void apply(const Representation& in, const BoundingBox& in_bbox, const Representation& out,
               const BoundingBox& out_bbox, Cropping&) const override {
        // do nothing
    }
};


static const IntersectBuilder<InputIntersectsOutput> INTERSECTS1("input-intersects-output");
static const IntersectBuilder<InputExtendsOutput> INTERSECTS2("input-extends-output");
static const IntersectBuilder<InputContainsOutputCheck> INTERSECTS3("input-contains-output-check");
static const IntersectBuilder<None> INTERSECTS4("none");


}  // namespace mir::util

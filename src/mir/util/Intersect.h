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


#pragma once

#include <iosfwd>
#include <memory>
#include <string>


namespace mir {
namespace method {
class Cropping;
}
namespace repres {
class Representation;
}
namespace util {
class BoundingBox;
}
}  // namespace mir


namespace mir::util {


namespace detail {


struct Intersect {
    using Representation = repres::Representation;
    using Cropping       = method::Cropping;

    Intersect()          = default;
    virtual ~Intersect() = default;

    Intersect(const Intersect&) = delete;
    Intersect(Intersect&&)      = delete;

    Intersect& operator=(const Intersect&) = delete;
    Intersect& operator=(Intersect&&)      = delete;

    virtual void apply(const Representation& in, const BoundingBox& in_bbox, const Representation& out,
                       const BoundingBox& out_bbox, Cropping&) const = 0;
};


}  // namespace detail


class IntersectFactory {
protected:
    explicit IntersectFactory(const std::string&);
    virtual ~IntersectFactory() = default;

public:
    virtual detail::Intersect* make() = 0;

    IntersectFactory(const IntersectFactory&) = default;
    IntersectFactory(IntersectFactory&&)      = default;

    IntersectFactory& operator=(const IntersectFactory&) = default;
    IntersectFactory& operator=(IntersectFactory&&)      = default;

    static detail::Intersect* build(const std::string&);
    static void list(std::ostream&);
};

template <class T>
class IntersectBuilder : public IntersectFactory {
    detail::Intersect* make() override { return new T; }

public:
    explicit IntersectBuilder(const std::string& name) : IntersectFactory(name) {}
};


struct Intersect final : detail::Intersect, std::unique_ptr<detail::Intersect> {
    void apply(const Representation& in, const BoundingBox& in_bbox, const Representation& out,
               const BoundingBox& out_bbox, Cropping& crop) const final {
        get()->apply(in, in_bbox, out, out_bbox, crop);
    }

    static Intersect build(const std::string& name) { return Intersect{IntersectFactory::build(name)}; }
    static void list(std::ostream& out) { IntersectFactory::list(out); }

protected:
    explicit Intersect(detail::Intersect*);
};


}  // namespace mir::util

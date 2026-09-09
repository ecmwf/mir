// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/search/PointSearch.h"


namespace eckit {
class JSON;
class MD5;
}  // namespace eckit

namespace mir::repres {
class Representation;
}  // namespace mir::repres


namespace mir::method::knn::pick {


class Pick {
public:
    using neighbours_t = std::vector<search::PointSearch::PointValueType>;

    Pick();
    virtual ~Pick();

    Pick(const Pick&) = delete;
    Pick(Pick&&)      = delete;

    Pick& operator=(const Pick&) = delete;
    Pick& operator=(Pick&&)      = delete;

    virtual void pick(const search::PointSearch&, const Point3&, neighbours_t&) const = 0;
    virtual size_t n() const                                                          = 0;
    virtual bool sameAs(const Pick&) const                                            = 0;
    virtual void hash(eckit::MD5&) const                                              = 0;

    virtual void distance(const repres::Representation&) const;

    virtual void json(eckit::JSON&) const = 0;

protected:
    const std::string& type() const;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Pick& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Pick& p) {
        p.json(s);
        return s;
    }
};


class PickFactory {
private:
    std::string name_;
    virtual Pick* make(const param::MIRParametrisation&) = 0;

protected:
    explicit PickFactory(const std::string& name);
    virtual ~PickFactory();

public:
    PickFactory(const PickFactory&)            = default;
    PickFactory(PickFactory&&)                 = default;
    PickFactory& operator=(const PickFactory&) = default;
    PickFactory& operator=(PickFactory&&)      = default;

    static const Pick* build(const std::string& name, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class PickBuilder : public PickFactory {
    Pick* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    explicit PickBuilder(const std::string& name) : PickFactory(name) {}
};


}  // namespace mir::method::knn::pick

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


#ifndef mir_method_knn_pick_Pick_h
#define mir_method_knn_pick_Pick_h

#include <iosfwd>

#include "mir/search/PointSearch.h"


namespace eckit {
class MD5;
}


namespace mir {
namespace method {
namespace knn {
namespace pick {


class Pick {
public:
    using neighbours_t = std::vector<search::PointSearch::PointValueType>;

    Pick();
    virtual ~Pick() override;

    Pick(const Pick&) = delete;
    Pick& operator=(const Pick&) = delete;

    virtual void pick(const search::PointSearch&, const Point3&, neighbours_t&) const = 0;
    virtual size_t n() const                                                          = 0;
    virtual bool sameAs(const Pick& other) const                                      = 0;
    virtual void hash(eckit::MD5&) const                                              = 0;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const Pick& p) {
        p.print(s);
        return s;
    }
};


class PickFactory {
private:
    std::string name_;
    virtual Pick* make(const param::MIRParametrisation&) = 0;

    PickFactory(const PickFactory&) = default;
    PickFactory& operator=(const PickFactory&) = default;

protected:
    PickFactory(const std::string& name);
    virtual ~PickFactory() override;

public:
    static const Pick* build(const std::string& name, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class PickBuilder : public PickFactory {
    virtual Pick* make(const param::MIRParametrisation& param) { return new T(param); }

public:
    PickBuilder(const std::string& name) : PickFactory(name) {}
};


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

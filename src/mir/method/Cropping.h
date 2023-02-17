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

#include "mir/util/BoundingBox.h"


namespace mir::method {


class Cropping {
public:
    Cropping();
    Cropping(const Cropping&);

    operator bool() const { return active_; }

    void hash(eckit::MD5&) const;
    bool sameAs(const Cropping&) const;

    void boundingBox(const util::BoundingBox&);
    const util::BoundingBox& boundingBox() const;

private:
    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const Cropping& p) {
        p.print(s);
        return s;
    }

    util::BoundingBox bbox_;
    bool active_;
};


}  // namespace mir::method

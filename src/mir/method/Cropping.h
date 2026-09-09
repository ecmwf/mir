// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/util/BoundingBox.h"


namespace mir::method {


class Cropping {
public:
    Cropping();

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

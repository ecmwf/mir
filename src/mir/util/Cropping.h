/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
/// @date May 2015


#ifndef mir_util_Cropping_h
#define mir_util_Cropping_h

#include <iosfwd>

#include "mir/util/BoundingBox.h"


namespace eckit {
class MD5;
}


namespace mir {
namespace util {


class Cropping {
public:

    Cropping();

    ~Cropping();

    void hash(eckit::MD5&) const;

    bool operator==(const Cropping& other) const;

    bool active() const;

    void boundingBox(const BoundingBox& bbox);
    const BoundingBox& boundingBox() const;

protected:

    void print(std::ostream&) const;

private:

    bool active_;
    BoundingBox bbox_;

    friend std::ostream& operator<<(std::ostream& s, const Cropping& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif


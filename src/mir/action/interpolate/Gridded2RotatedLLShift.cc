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

#include "mir/action/interpolate/Gridded2RotatedLLShift.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"


#include "mir/repres/latlon/RotatedLLShift.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RotatedLLShift::Gridded2RotatedLLShift(const param::MIRParametrisation &parametrisation):
  Gridded2GriddedInterpolation(parametrisation) {

  std::vector<double> value;
  ASSERT(parametrisation_.get("user.grid", value));
  ASSERT(value.size() == 2);
  increments_ = util::Increments(value[0], value[1]);

  ASSERT(parametrisation_.get("user.rotation", value));
  ASSERT(value.size() == 2);

  rotation_ = util::Rotation(value[0], value[1]);

  ASSERT(parametrisation_.get("user.shift", value));
  ASSERT(value.size() == 2);

  shift_ = util::Shift(value[0], value[1]);

}


Gridded2RotatedLLShift::~Gridded2RotatedLLShift() {
}


bool Gridded2RotatedLLShift::sameAs(const Action& other) const {
  const Gridded2RotatedLLShift* o = dynamic_cast<const Gridded2RotatedLLShift*>(&other);
  return o && (increments_ == o->increments_)
         && (rotation_ == o->rotation_)
         && (shift_ == o->shift_);
}

void Gridded2RotatedLLShift::print(std::ostream &out) const {
  out << "Gridded2RotatedLLShift[increments=" << increments_
      << ",rotation=" << rotation_
      << ",shift=" << shift_ << "]";
}


const repres::Representation *Gridded2RotatedLLShift::outputRepresentation() const {

  eckit::Fraction we(increments_.west_east());
  eckit::Fraction sn(increments_.south_north());

  double north = 90 - (90 / sn).decimalPart() * sn;
  double south = -90 - (-90 / sn).decimalPart() * sn;

  double west = 0;
  double east = 360 - (360 / we).decimalPart() * we;

  util::BoundingBox bbox(north,
                         west,
                         south,
                         east - increments_.west_east());

  return new repres::latlon::RotatedLLShift(
           bbox,
           increments_,
           rotation_,
           shift_);
}


namespace {
static ActionBuilder< Gridded2RotatedLLShift > grid2grid("interpolate.grid2rotated-regular-ll-shift");
}


}  // namespace action
}  // namespace mir


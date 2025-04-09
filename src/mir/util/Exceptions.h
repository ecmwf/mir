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

#include <sstream>

#include "eckit/exception/Exceptions.h"

#define ASSERT_KEYWORD_GRID_SIZE(size) ASSERT_MSG(size == 2, "keyword 'grid' expected size=2")
#define ASSERT_KEYWORD_AREA_SIZE(size) ASSERT_MSG(size == 4, "keyword 'area' expected size=4")
#define ASSERT_KEYWORD_ROTATION_SIZE(size) ASSERT_MSG(size == 2, "keyword 'rotation' expected size=2")

#define ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT(m, size, count) \
    ASSERT_MSG((size) == (count), m ": values size equals iterator count")

#define ASSERT_VALUES_SIZE_EQ_NUMBER_OF_COEFFS(m, size, count) \
    ASSERT_MSG((size) == (count), m ": values size equals number of coefficients")

#define ASSERT_NONEMPTY_INTERPOLATION(m, nonempty) \
    ASSERT_MSG((nonempty), m ": non-empty interpolation (to at least one point)")

#define ASSERT_NONEMPTY_AREA(m, nonempty) ASSERT_MSG((nonempty), m ": non-empty area crop/mask (to at least one point)")


namespace mir::exception {


using eckit::BadValue;
using eckit::CantOpenFile;
using eckit::FailedSystemCall;
using eckit::FunctionalityNotSupported;
using eckit::ReadError;
using eckit::SeriousBug;
using eckit::UserError;
using eckit::WriteError;


class CannotConvert : public eckit::Exception {
public:
    template <class T>
    CannotConvert(const char* from, const char* to, const std::string& name, const T& value) {
        std::ostringstream os;
        os << "Cannot convert " << value << " from " << from << " to " << to << " (requesting " << name << ")";
        reason(os.str());
    }
};


class InvalidWeightMatrix : public eckit::Exception {
public:
    InvalidWeightMatrix(const char* when, const std::string& what) {
        std::ostringstream os;
        os << "Invalid weight matrix (" << when << "): " << what;
        reason(os.str());
    }
};


}  // namespace mir::exception

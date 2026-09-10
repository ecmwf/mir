// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/plan/Action.h"


namespace mir {
namespace util {
class Bitmap;
}
namespace action {


class BitmapFilter : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit BitmapFilter(const param::MIRParametrisation&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::string path_;

    // -- Methods

    util::Bitmap& bitmap() const;

    // -- Overridden methods

    void custom(std::ostream&) const override;
    void print(std::ostream&) const override;

    void execute(context::Context&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace action
}  // namespace mir

// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/search/tree/TreeMapped.h"


namespace mir::search::tree {


class TreeMappedAnonymousMemory : public TreeMapped {

    bool ready() const override { return false; }

    void commit() override {}

    void print(std::ostream& out) const override { out << "TreeMappedAnonymousMemory[]"; }

public:
    TreeMappedAnonymousMemory(const repres::Representation&);
};


}  // namespace mir::search::tree

// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/search/tree/TreeMappedAnonymousMemory.h"


namespace mir::search::tree {


TreeMappedAnonymousMemory::TreeMappedAnonymousMemory(const repres::Representation& r) : TreeMapped(r, "/dev/zero") {}


static const TreeBuilder<TreeMappedAnonymousMemory> builder("mapped-anonymous-memory");


}  // namespace mir::search::tree

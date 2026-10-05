// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "eckit/filesystem/PathName.h"
#include "eckit/memory/Shmget.h"
#include "eckit/testing/Test.h"

#include "mir/caching/SharedMemoryKey.h"
#include "mir/caching/legendre/SharedMemoryLoader.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::tests::unit {


CASE("SharedMemoryLoader unloads, for loader tmp-* from environment") {
    // $MIR_LEGENDRE_LOADER=tmp-shmem (see CMakeLists.txt), any file can be loaded
    eckit::PathName path("legendre_shared_memory_loader.cc");
    param::SimpleParametrisation param;

    {
        caching::legendre::SharedMemoryLoader loader(param, path);
    }

    EXPECT(eckit::Shmget::shmget(caching::shared_memory_key(path.realName()), 0, 0600) < 0);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

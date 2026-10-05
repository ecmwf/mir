// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <string>
#include <utility>
#include <vector>

#include "eckit/testing/Test.h"

#include "mir/api/MIRJob.h"
#include "mir/caching/legendre/FileLoader.h"
#include "mir/caching/legendre/SharedMemoryLoader.h"
#include "mir/caching/matrix/FileLoader.h"
#include "mir/caching/matrix/SharedMemoryLoader.h"
#include "mir/input/RawInput.h"
#include "mir/output/ResizableOutput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::tests::unit {


// Loader counting its constructions, i.e. how many times a cache file is loaded
template <class Loader>
struct Counted : Loader {
    template <typename... Args>
    explicit Counted(Args&&... args) : Loader(std::forward<Args>(args)...) {
        ++loads;
    }

    static size_t loads;
};

template <class Loader>
size_t Counted<Loader>::loads = 0;


// Selected by $MIR_LEGENDRE_LOADER and $MIR_MATRIX_LOADER (see CMakeLists.txt)
static const caching::legendre::LegendreLoaderBuilder<Counted<caching::legendre::FileLoader>> LEGENDRE_FILE_IO(
    "counted-file-io");
static const caching::legendre::LegendreLoaderBuilder<Counted<caching::legendre::SharedMemoryLoader>> LEGENDRE_SHMEM(
    "tmp-counted-shmem");
static const caching::matrix::MatrixLoaderBuilder<Counted<caching::matrix::FileLoader>> MATRIX_FILE_IO(
    "counted-file-io");
static const caching::matrix::MatrixLoaderBuilder<Counted<caching::matrix::SharedMemoryLoader>> MATRIX_SHMEM(
    "tmp-counted-shmem");


size_t legendre_loads() {
    return Counted<caching::legendre::FileLoader>::loads + Counted<caching::legendre::SharedMemoryLoader>::loads;
}


size_t matrix_loads() {
    return Counted<caching::matrix::FileLoader>::loads + Counted<caching::matrix::SharedMemoryLoader>::loads;
}


void execute(const api::MIRJob& job, param::SimpleParametrisation& meta, std::vector<double>&& values) {
    input::RawInput input(values.data(), values.size(), meta);

    std::vector<double> result;
    param::SimpleParametrisation result_meta;
    output::ResizableOutput output(result, result_meta);

    job.execute(input, output);
}


// In-memory cache capacities are set (see CMakeLists.txt) below the footprint of the cache files


CASE("Legendre coefficients (T10 to O9) loaded once") {
    param::SimpleParametrisation meta;
    meta.set("spectral", true);
    meta.set("gridType", "sh");
    meta.set("truncation", 10);

    api::MIRJob job;
    job.set("grid", "O9");
    job.set("intgrid", "none");

    execute(job, meta, std::vector<double>(11 * 12, 0.));
    EXPECT_EQUAL(legendre_loads(), 1);

    execute(job, meta, std::vector<double>(11 * 12, 0.));
    EXPECT_EQUAL(legendre_loads(), 1);
}


CASE("Interpolation matrix (O9 to 10/10) loaded once") {
    std::vector<long> pl{20, 24, 28, 32, 36, 40, 44, 48, 52, 52, 48, 44, 40, 36, 32, 28, 24, 20};

    param::SimpleParametrisation meta;
    meta.set("gridded", true);
    meta.set("gridType", "reduced_gg");
    meta.set("north", 90.);
    meta.set("west", 0.);
    meta.set("south", -90.);
    meta.set("east", 360.);
    meta.set("N", 9);
    meta.set("pl", pl);

    api::MIRJob job;
    job.set("grid", std::vector<double>{10., 10.});

    execute(job, meta, std::vector<double>(648, 0.));
    EXPECT_EQUAL(matrix_loads(), 1);

    execute(job, meta, std::vector<double>(648, 0.));
    EXPECT_EQUAL(matrix_loads(), 1);
}


}  // namespace mir::tests::unit


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}

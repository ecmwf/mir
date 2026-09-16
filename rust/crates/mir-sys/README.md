# mir-sys

Rust bindings to ECMWF's [mir](https://github.com/ecmwf/mir) (Meteorological Interpolation and Regridding) C++ library.

This crate provides raw FFI bindings using [cxx](https://cxx.rs/).

## Building

You need:

- Rust 1.90 or newer ([rustup.rs](https://rustup.rs))
- A C++17 compiler and CMake 3.14 or newer
- git, and an SSH key that can read the ecmwf GitHub repositories

Then, from the repository root:

```sh
cd rust
cargo build
```

That is all. By default the build compiles mir and everything it needs (eckit,
ecCodes, atlas, metkit) from source, so the first build downloads a lot and
takes a while; later builds reuse it. When the crate is inside a mir checkout it
builds that checkout, so local C++ changes are picked up.

Run the tests and the example:

```sh
cargo test
cargo run --example regrid -- path/to/input.grib
```

### Using an installed mir

To link against an existing installation instead of building from source:

```sh
cargo build --no-default-features --features system
```

CMake locates the libraries through `CMAKE_PREFIX_PATH`.

## Features

### Build strategy (mutually exclusive)

- `vendored` - Build mir and its dependencies (eckit, metkit, ecCodes, atlas) from source.
- `system` - Link against system-installed mir.

`vendored` is enabled by default.

## License

Apache-2.0

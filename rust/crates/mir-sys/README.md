# mir-sys

Rust bindings to ECMWF's [mir](https://github.com/ecmwf/mir) (Meteorological Interpolation and Regridding) C++ library.

This crate provides raw FFI bindings using [cxx](https://cxx.rs/).

## Features

### Build strategy (mutually exclusive)

- `vendored` - Build mir and its dependencies (eckit, metkit, ecCodes, atlas) from source.
- `system` - Link against system-installed mir.

`vendored` is enabled by default.

## License

Apache-2.0

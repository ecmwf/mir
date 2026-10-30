<!--
SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
SPDX-License-Identifier: Apache-2.0
-->

mir
===

[![codecov](https://codecov.io/gh/ecmwf/mir/branch/develop/graph/badge.svg)](https://app.codecov.io/gh/ecmwf/mir)
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/ecmwf/mir/blob/develop/LICENSE)
[![REUSE status](https://api.reuse.software/badge/github.com/ecmwf/mir)](https://api.reuse.software/info/github.com/ecmwf/mir)

Land-sea masks
--------------

Interpolation methods using land-sea masks (e.g. `--interpolation=nearest-lsm`, or `--lsm`) require mask files, configured in `etc/mir/lsm.yaml` under `~mir/share/mir/masks/`. These files are large (the 1km masks are over 100 MB each), so their download at configure time can be disabled with `-DENABLE_MIR_DOWNLOAD_MASKS=OFF` (disabling as well the tests requiring them).

When a mask file is not installed it is downloaded at runtime, on first use, and cached locally. This is controlled by the environment variables:

- `MIR_HOME_URL`: base URL of the remote repository the masks are downloaded from (default `https://sites.ecmwf.int/repository/mir`); a mask `~mir/share/mir/masks/<file>` is downloaded from `$MIR_HOME_URL/share/mir/masks/<file>`, so this can point to a mirror (e.g. for sites without internet access)
- `MIR_CACHE_PATH`: local cache directory (default `/tmp/cache`), masks are stored under `$MIR_CACHE_PATH/mir/masks/`; prefer a persistent location, or place the mask files there in advance
- `MIR_CACHING`: caching is enabled by default; setting `MIR_CACHING=0` also disables the runtime download, so mask files not installed cannot be resolved

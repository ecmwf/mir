# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import os
import shutil
import tempfile

_cache_path = None


def pytest_configure(config):
    """eckit::geo and mir cache to a temporary directory, not the runtime cache (set before it is resolved, on first use)"""
    global _cache_path
    _cache_path = tempfile.mkdtemp(prefix="mir_cache.")
    os.environ["ECKIT_GEO_CACHE_PATH"] = os.path.join(_cache_path, "eckit-geo")
    os.environ["MIR_CACHE_PATH"] = os.path.join(_cache_path, "mir")


def pytest_unconfigure(config):
    if _cache_path is not None:
        shutil.rmtree(_cache_path, ignore_errors=True)

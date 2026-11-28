# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0


import numpy as np
import pytest

import mir

GRID = {"grid": [30, 30]}  # 7 x 12 points
SIZE = 7 * 12


def passthrough(values):
    out = mir.ArrayOutput()
    mir.Job().execute(mir.ArrayInput(values, GRID), out)
    return out.values().ravel()


@pytest.mark.parametrize("dtype", [np.float64, np.float32])
def test_nan_is_missing(dtype):
    values = np.arange(SIZE, dtype=dtype)
    values[[0, 5, SIZE - 1]] = np.nan

    result = passthrough(values)
    missing = np.isnan(values)
    assert np.array_equal(np.isnan(result), missing)
    assert np.array_equal(result[~missing], values[~missing])


@pytest.mark.parametrize("dtype", [np.float64, np.float32])
def test_no_missing_values(dtype):
    # 9999 is the default missing value, but here a valid value
    values = np.arange(SIZE, dtype=dtype)
    values[3] = 9999

    result = passthrough(values)
    assert not np.isnan(result).any()
    assert np.array_equal(result, values)


if __name__ == "__main__":
    pytest.main([__file__])

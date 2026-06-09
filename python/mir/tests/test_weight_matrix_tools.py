# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import sys

import numpy as np
import pytest
from scipy.sparse import eye

pytest.importorskip("mir", exc_type=(ImportError, OSError))
from mir.tools.weight_matrix_convert import _load
from mir.tools.weight_matrix_convert import _save
from mir.tools.weight_matrix_convert import main as convert_main
from mir.weight_matrix import WeightMatrix


@pytest.fixture
def wm():
    return WeightMatrix.from_csr(eye(4, format="csr"))


@pytest.fixture
def mat_file(tmp_path, wm):
    return wm.to_mat(tmp_path / "source")


@pytest.fixture
def npz_file(tmp_path, wm):
    return wm.to_npz(tmp_path / "source")


def test_load_mat(mat_file, wm):
    loaded = _load(mat_file)
    np.testing.assert_allclose(loaded.to_csr().toarray(), wm.to_csr().toarray())


def test_load_npz(npz_file, wm):
    loaded = _load(npz_file)
    np.testing.assert_allclose(loaded.to_csr().toarray(), wm.to_csr().toarray())


def test_load_unsupported_raises(tmp_path):
    with pytest.raises(ValueError, match="Unsupported input"):
        _load(tmp_path / "file.txt")


def test_save_mat(tmp_path, wm):
    out = _save(wm, tmp_path / "out.mat")
    assert out.suffix == ".mat"
    back = WeightMatrix.from_mat(out)
    np.testing.assert_allclose(back.to_csr().toarray(), wm.to_csr().toarray())


def test_save_npz(tmp_path, wm):
    out = _save(wm, tmp_path / "out.npz")
    assert out.suffix == ".npz"
    back = WeightMatrix.from_npz(out)
    np.testing.assert_allclose(back.to_csr().toarray(), wm.to_csr().toarray())


def test_save_unsupported_raises(tmp_path, wm):
    with pytest.raises(ValueError, match="Unsupported output"):
        _save(wm, tmp_path / "out.csv")


def test_convert_main_mat_to_npz(tmp_path, mat_file, wm, monkeypatch):
    out = tmp_path / "out.npz"
    monkeypatch.setattr(sys, "argv", ["mir-weight-matrix-convert", str(mat_file), str(out)])
    convert_main()
    np.testing.assert_allclose(
        WeightMatrix.from_npz(out).to_csr().toarray(),
        wm.to_csr().toarray(),
    )


def test_convert_main_npz_to_mat(tmp_path, npz_file, wm, monkeypatch):
    out = tmp_path / "out.mat"
    monkeypatch.setattr(sys, "argv", ["mir-weight-matrix-convert", str(npz_file), str(out)])
    convert_main()
    np.testing.assert_allclose(
        WeightMatrix.from_mat(out).to_csr().toarray(),
        wm.to_csr().toarray(),
    )

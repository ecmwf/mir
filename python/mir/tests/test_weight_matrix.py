# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import numpy as np
import pytest
from scipy.sparse import csr_array
from scipy.sparse import csr_matrix
from scipy.sparse import eye

pytest.importorskip("mir", exc_type=(ImportError, OSError))
from mir.weight_matrix import WeightMatrix


@pytest.fixture
def simple():
    # 3x4:  row0=[1,0,2,0]  row1=[0,3,0,0]  row2=[0,0,0,4]
    data    = np.array([1.0, 2.0, 3.0, 4.0])
    indices = np.array([0, 2, 1, 3])
    indptr  = np.array([0, 2, 3, 4])
    return csr_array((data, indices, indptr), shape=(3, 4))


@pytest.fixture
def identity():
    return csr_array(eye(5, format="csr"))


def test_from_csr_passthrough(simple):
    wm = WeightMatrix.from_csr(simple)
    assert isinstance(wm.to_csr(), csr_array)
    np.testing.assert_array_equal(wm.to_csr().toarray(), simple.toarray())


def test_from_csr_matrix(simple):
    wm = WeightMatrix.from_csr(csr_matrix(simple))
    assert isinstance(wm.to_csr(), csr_array)
    np.testing.assert_array_equal(wm.to_csr().toarray(), simple.toarray())


def test_from_csr_invalid():
    with pytest.raises(TypeError):
        WeightMatrix(np.eye(3))


def test_repr(simple):
    wm = WeightMatrix.from_csr(simple)
    assert "WeightMatrix" in repr(wm)
    assert "3, 4" in repr(wm)


def test_mat_roundtrip(tmp_path, simple):
    wm = WeightMatrix.from_csr(simple)
    out = wm.to_mat(tmp_path / "test")          # no extension provided
    assert out.suffix == ".mat"
    back = WeightMatrix.from_mat(out)
    np.testing.assert_allclose(back.to_csr().toarray(), simple.toarray())


def test_mat_roundtrip_extension_enforced(tmp_path, simple):
    wm = WeightMatrix.from_csr(simple)
    out = wm.to_mat(tmp_path / "test.npz")      # wrong extension — must be corrected
    assert out.suffix == ".mat"


def test_mat_roundtrip_identity(tmp_path, identity):
    wm = WeightMatrix.from_csr(identity)
    back = WeightMatrix.from_mat(wm.to_mat(tmp_path / "id"))
    np.testing.assert_allclose(back.to_csr().toarray(), identity.toarray())


def test_npz_roundtrip(tmp_path, simple):
    wm = WeightMatrix.from_csr(simple)
    out = wm.to_npz(tmp_path / "test")          # no extension provided
    assert out.suffix == ".npz"
    back = WeightMatrix.from_npz(out)
    np.testing.assert_allclose(back.to_csr().toarray(), simple.toarray())


def test_npz_roundtrip_extension_enforced(tmp_path, simple):
    wm = WeightMatrix.from_csr(simple)
    out = wm.to_npz(tmp_path / "test.mat")      # wrong extension — must be corrected
    assert out.suffix == ".npz"


def test_modify_row_multiply_write(tmp_path):
    # 3x3:  row0=[1,0,2]  row1=[0,3,0]  row2=[0,0,4]
    data    = np.array([1.0, 2.0, 3.0, 4.0])
    indices = np.array([0, 2, 1, 2])
    indptr  = np.array([0, 2, 3, 4])
    original = csr_array((data, indices, indptr), shape=(3, 3))

    wm = WeightMatrix.from_csr(original)

    # modify row 0: add weight at col 1 (0→0.5), zero existing weight at col 2 (2→0)
    lil = wm.to_csr().tolil()
    lil[0, 1] = 0.5
    lil[0, 2] = 0.0
    modified = csr_array(lil)
    modified.eliminate_zeros()

    wm2 = WeightMatrix.from_csr(modified)

    # matrix × vector
    v = np.array([1.0, 2.0, 3.0])
    result = wm2.to_csr() @ v
    # row0: [1, 0.5, 0] · [1,2,3] = 1 + 1 = 2
    # row1: [0, 3,   0] · [1,2,3] = 6
    # row2: [0, 0,   4] · [1,2,3] = 12
    np.testing.assert_allclose(result, [2.0, 6.0, 12.0])

    # write both formats, read back, verify
    mat_path = wm2.to_mat(tmp_path / "modified")
    npz_path = wm2.to_npz(tmp_path / "modified")

    from_mat = WeightMatrix.from_mat(mat_path)
    from_npz = WeightMatrix.from_npz(npz_path)

    np.testing.assert_allclose(from_mat.to_csr().toarray(), modified.toarray())
    np.testing.assert_allclose(from_npz.to_csr().toarray(), modified.toarray())

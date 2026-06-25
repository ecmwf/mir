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
from mir.weight_matrix import WeightMatrix
from scipy import sparse

import mir

GRIDS = [
    "1/1",
    dict(grid=[2, 2]),
    "H4n",
    "ORCA2_T",
    dict(latitudes=[1, 2, 3], longitudes=[4, 5, 6]),
]


@pytest.fixture
def simple():
    # 3x4:  row0=[1,0,2,0]  row1=[0,3,0,0]  row2=[0,0,0,4]
    data = np.array([1.0, 2.0, 3.0, 4.0])
    indices = np.array([0, 2, 1, 3])
    indptr = np.array([0, 2, 3, 4])
    return sparse.csr_array((data, indices, indptr), shape=(3, 4))


@pytest.fixture
def identity():
    return sparse.csr_array(sparse.eye(5, format="csr"))


def test_from_csr_passthrough(simple):
    wm = WeightMatrix.from_csr(simple)
    assert isinstance(wm.to_csr(), sparse.csr_array)
    np.testing.assert_array_equal(wm.to_csr().toarray(), simple.toarray())


def test_from_csr_matrix(simple):
    wm = WeightMatrix.from_csr(sparse.csr_matrix(simple))
    assert isinstance(wm.to_csr(), sparse.csr_array)
    np.testing.assert_array_equal(wm.to_csr().toarray(), simple.toarray())


def test_from_csr_invalid():
    with pytest.raises(TypeError):
        WeightMatrix(np.eye(3))


def test_repr(simple):
    wm = WeightMatrix.from_csr(simple)
    assert "WeightMatrix" in repr(wm)
    assert "3, 4" in repr(wm)


@pytest.mark.parametrize("ext", [".mat", ".npz"])
def test_file_roundtrip(tmp_path, simple, ext):
    wm = WeightMatrix.from_csr(simple)
    out = wm.to_file(tmp_path / f"test{ext}")
    assert out.suffix == ext
    back = WeightMatrix.from_file(out)
    np.testing.assert_allclose(back.to_csr().toarray(), simple.toarray())


def test_from_file_unsupported_raises(tmp_path):
    with pytest.raises(ValueError, match="from_file"):
        WeightMatrix.from_file(tmp_path / "weights.txt")


def test_to_file_unsupported_raises(tmp_path, simple):
    wm = WeightMatrix.from_csr(simple)
    with pytest.raises(ValueError, match="to_file"):
        wm.to_file(tmp_path / "weights.txt")


def test_mat_roundtrip_identity(tmp_path, identity):
    wm = WeightMatrix.from_csr(identity)
    back = WeightMatrix.from_mat(wm.to_mat(tmp_path / "id"))
    np.testing.assert_allclose(back.to_csr().toarray(), identity.toarray())


def test_edit_with_lil_rows_data_and_roundtrip(tmp_path):
    """Instructive workflow: edit rows in LIL, convert back, apply, and persist."""

    # interpolation-like 4x5 matrix
    # [  .   0.6   .   0.4   . ]
    # [  .    .   1.0   .    . ]
    # [ 0.3   .    .    .   0.7]
    # [  .   0.2  0.5  0.3   . ]
    data = np.array([0.6, 0.4, 1.0, 0.3, 0.7, 0.2, 0.5, 0.3])
    indices = np.array([1, 3, 2, 0, 4, 1, 2, 3])
    indptr = np.array([0, 2, 3, 5, 8])
    wm = WeightMatrix.from_csr(sparse.csr_array((data, indices, indptr), shape=(4, 5)))

    # Edit row 2 using direct row/data lists.
    m = wm.to_lil()
    m.rows[2] = [0]
    m.data[2] = [1.0]
    wm2 = WeightMatrix.from_lil(m)

    field = np.array([1.0, 2.0, 3.0, 4.0, 5.0])
    result = wm2.to_csr() @ field
    np.testing.assert_allclose(result, [2.8, 3.0, 1.0, 3.1])

    # Update the same row again: 0.1*field[0] + 0.9*field[4] = 4.6
    m = wm2.to_lil()
    m.rows[2] = [0, 4]
    m.data[2] = [0.1, 0.9]
    wm3 = WeightMatrix.from_lil(m)

    result2 = wm3.to_csr() @ field
    np.testing.assert_allclose(result2[2], 4.6)

    # unified file API roundtrip
    written = wm3.to_file(tmp_path / "weights_demo.npz")
    reloaded = WeightMatrix.from_file(written)
    np.testing.assert_allclose(reloaded.to_csr().toarray(), wm3.to_csr().toarray())


def test_weight_matrix_file_helpers(tmp_path):
    wm = WeightMatrix.from_csr(sparse.csr_array(sparse.eye(4, format="csr")))
    src = wm.to_file(tmp_path / "source.mat")
    loaded = WeightMatrix.from_file(src)
    np.testing.assert_allclose(loaded.to_csr().toarray(), wm.to_csr().toarray())


@pytest.mark.parametrize("src_ext,dst_ext", [(".mat", ".npz"), (".npz", ".mat")])
def test_file_convert_roundtrip(tmp_path, src_ext, dst_ext):
    wm = WeightMatrix.from_csr(sparse.csr_array(sparse.eye(4, format="csr")))
    src = wm.to_file(tmp_path / f"src{src_ext}")
    dst = WeightMatrix.from_file(src).to_file(tmp_path / f"dst{dst_ext}")
    np.testing.assert_allclose(WeightMatrix.from_file(dst).to_csr().toarray(), wm.to_csr().toarray())


@pytest.mark.parametrize("input_grid", ["H4n", {"grid": "H4n"}])
@pytest.mark.parametrize("output_grid", ["O4", {"grid": "O4"}])
def test_from_regrid_and_persist(tmp_path, input_grid, output_grid):
    out = tmp_path / "weights.npz"

    wm = WeightMatrix.from_regrid(
        in_grid=input_grid,
        out_grid=output_grid,
        interpolation="nearest-neighbour",
        nclosest=1,
    )
    wm.to_file(out)

    assert isinstance(wm, WeightMatrix)
    assert out.exists()


def test_from_regrid_invalid_grid_type_raises():
    with pytest.raises(TypeError, match="Expected grid"):
        WeightMatrix.from_regrid(
            in_grid=object(),
            out_grid={"grid": "O32"},
            interpolation="nearest-neighbour",
        )


@pytest.mark.parametrize("in_grid, out_grid", [(a, b) for a in GRIDS for b in GRIDS if a != b])
def test_make_matrix_with_spec_str(tmp_path, in_grid, out_grid):
    in_grid_obj = mir.Grid(in_grid)
    out_grid_obj = mir.Grid(out_grid)

    output_file = tmp_path / "weights.npz"

    result = WeightMatrix.from_regrid(
        in_grid=in_grid_obj.spec_str,
        out_grid=out_grid_obj.spec_str,
        interpolation="nearest-neighbour",
        nclosest=1,
    )
    result.to_file(output_file)

    assert output_file.exists()
    assert isinstance(result, WeightMatrix)

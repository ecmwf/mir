# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.


import sys
import tempfile
from pathlib import Path

import numpy as np
import yaml
from eckit.stream import Stream
from scipy.sparse import csr_array
from scipy.sparse import issparse
from scipy.sparse import load_npz
from scipy.sparse import save_npz

# ---- private I/O helpers ----


def _dtype_uint(little_endian: bool, size: int) -> np.dtype:
    return np.dtype({4: np.uint32, 8: np.uint64}[size]).newbyteorder("<" if little_endian else ">")


def _dtype_float(little_endian: bool, size: int) -> np.dtype:
    return np.dtype({4: np.float32, 8: np.float64}[size]).newbyteorder("<" if little_endian else ">")


def _to_csr(matrix) -> csr_array:
    if isinstance(matrix, csr_array):
        return matrix
    if issparse(matrix):
        return csr_array(matrix)
    raise TypeError(f"Expected scipy sparse matrix or array, got {type(matrix)}")


def _to_spec_str(spec) -> str:
    if isinstance(spec, str):
        return spec

    spec_str = getattr(spec, "spec_str", None)
    if isinstance(spec_str, str):
        return spec_str

    if isinstance(spec, dict):
        return yaml.dump(spec, default_flow_style=True).strip()

    try:
        import mir

        return mir.Grid(spec).spec_str
    except Exception as e:
        raise TypeError("Expected grid to be a spec string, dict, or mir.Grid-compatible value") from e


def _path_with_supported_suffix(path, *, where: str) -> tuple[Path, str]:
    p = Path(path)
    suffix = p.suffix.lower()
    if suffix not in (".mat", ".npz"):
        raise ValueError(f"{where}: file extension must be .mat or .npz")
    return p, suffix


def _read_mat(path) -> csr_array:
    with open(path, "rb") as f:
        s = Stream(f)
        rows = s.read_unsigned_long()
        cols = s.read_unsigned_long()
        s.read_unsigned_long()  # non-zeros (inferred from blobs)

        little_endian = s.read_int() != 0
        index_item_size = s.read_unsigned_long()
        scalar_item_size = s.read_unsigned_long()
        s.read_unsigned_long()  # sizeof(size), ignored

        outer = np.frombuffer(s.read_large_blob(), dtype=_dtype_uint(little_endian, index_item_size))
        inner = np.frombuffer(s.read_large_blob(), dtype=_dtype_uint(little_endian, index_item_size))
        data = np.frombuffer(s.read_large_blob(), dtype=_dtype_float(little_endian, scalar_item_size))

        return csr_array((data, inner, outer), shape=(rows, cols))


def _write_mat(path, matrix: csr_array) -> None:
    m = matrix.astype(np.float64)
    m.sort_indices()

    little_endian = sys.byteorder == "little"
    order = "<" if little_endian else ">"
    index_dtype = np.dtype(np.uint32).newbyteorder(order)
    scalar_dtype = np.dtype(np.float64).newbyteorder(order)

    outer = m.indptr.astype(index_dtype)
    inner = m.indices.astype(index_dtype)
    data = m.data.astype(scalar_dtype)

    with open(path, "wb") as f:
        s = Stream(f)
        s.write_unsigned_long(m.shape[0])
        s.write_unsigned_long(m.shape[1])
        s.write_unsigned_long(m.nnz)
        s.write_int(int(little_endian))
        s.write_unsigned_long(index_dtype.itemsize)
        s.write_unsigned_long(scalar_dtype.itemsize)
        s.write_unsigned_long(8)  # sizeof(size) = uint64
        s.write_large_blob(outer.tobytes())
        s.write_large_blob(inner.tobytes())
        s.write_large_blob(data.tobytes())


def _read_npz(path) -> csr_array:
    return csr_array(load_npz(path))


def _write_npz(path, matrix: csr_array) -> None:
    save_npz(path, matrix)


# ---- public class ----


class WeightMatrix:
    """Sparse interpolation weight matrix backed by a scipy CSR array."""

    def __init__(self, matrix):
        self._m = _to_csr(matrix)

    # --- constructors ---

    @classmethod
    def from_csr(cls, matrix) -> "WeightMatrix":
        """Construct from any scipy sparse matrix/array."""
        return cls(matrix)

    @classmethod
    def from_lil(cls, matrix) -> "WeightMatrix":
        """Construct from a scipy LIL sparse matrix/array."""
        return cls(csr_array(matrix))

    @classmethod
    def from_file(cls, path) -> "WeightMatrix":
        """Load from a .mat or .npz file."""
        p, suffix = _path_with_supported_suffix(path, where="from_file")
        if suffix == ".mat":
            return cls(_read_mat(p))
        return cls(_read_npz(p))

    @classmethod
    def from_regrid(
        cls,
        in_grid,
        out_grid,
        kwargs=None,
        **job_kwargs,
    ) -> "WeightMatrix":
        """Build interpolation weights from in_grid to out_grid using mir regridding."""
        import mir

        options = {}
        if kwargs is not None:
            options.update(kwargs)
        options.update(job_kwargs)

        job = mir.Job()
        job.set("grid", _to_spec_str(out_grid))

        with tempfile.TemporaryDirectory() as tmpdir:
            mat_path = Path(tmpdir) / "weights.mat"

            job.set("interpolation-matrix", str(mat_path))
            grid_input = mir.GridSpecInput(_to_spec_str(in_grid))

            for key, val in options.items():
                job.set(key, val)

            try:
                job.execute(grid_input, mir.EmptyOutput())
            except Exception as e:
                raise RuntimeError(f"from_regrid: error: {e}.") from e

            if not mat_path.exists():
                raise FileNotFoundError(f"from_regrid: matrix file '{mat_path}' not found.")

            return cls.from_mat(mat_path)

    @classmethod
    def from_mat(cls, path) -> "WeightMatrix":
        """Load from eckit cached matrix .mat file."""
        return cls(_read_mat(Path(path).with_suffix(".mat")))

    @classmethod
    def from_npz(cls, path) -> "WeightMatrix":
        """Load from scipy .npz file."""
        return cls(_read_npz(Path(path).with_suffix(".npz")))

    # --- serialisation ---

    def to_csr(self) -> csr_array:
        """Return the underlying scipy CSR array."""
        return self._m

    def to_lil(self):
        """Return the matrix as a scipy LIL sparse matrix."""
        return self._m.tolil()

    def to_file(self, path) -> Path:
        """Write to a .mat or .npz file."""
        p, suffix = _path_with_supported_suffix(path, where="to_file")
        if suffix == ".mat":
            _write_mat(p, self._m)
        else:
            _write_npz(p, self._m)
        return p

    def to_mat(self, path) -> Path:
        """Write to eckit cached matrix .mat (extension enforced)."""
        p = Path(path).with_suffix(".mat")
        return self.to_file(p)

    def to_npz(self, path) -> Path:
        """Write to scipy .npz (extension enforced)."""
        p = Path(path).with_suffix(".npz")
        return self.to_file(p)

    # --- properties ---

    @property
    def shape(self):
        return self._m.shape

    @property
    def nnz(self):
        return self._m.nnz

    def __repr__(self):
        return f"WeightMatrix(shape={self.shape}, nnz={self.nnz})"

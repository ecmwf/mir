cimport mir_defs as mir
from cython.operator cimport dereference

from mir._spec import from_json
from mir._spec import mir_options
from mir._spec import to_yaml

# interpolation types (as mir methods)
METHODS = {
    "linear": "linear",
    "bilinear": "bilinear",
    "grid-box-average": "grid-box-average",
    "grid-box-statistics": "grid-box-statistics",
    "healpix-resample": "healpix-resample",
    "healpix-statistics": "healpix-statistics",
    "nn": "nearest-neighbour",
    "k-nn": "k-nearest",
    "voronoi-statistics": "voronoi-statistics",
    "structured-bilinear": "structured-bilinear",
    "structured-bicubic": "structured-bicubic",
    "structured-biquasicubic": "structured-biquasicubic",
}

ALIASES = {
    "nearest-neighbour": "nn",
    "nearest-neighbor": "nn",
    "k-nearest": "k-nn",
    "k-nearest-neighbours": "k-nn",
    "k-nearest-neighbors": "k-nn",
}

# statistics of the *-statistics types
STATISTICS = (
    "count",
    "count-above-upper-limit",
    "count-below-lower-limit",
    "maximum",
    "minimum",
    "mean",
    "variance",
    "skewness",
    "kurtosis",
    "stddev",
    "sum",
    "mode-real",
    "mode-integral",
    "mode-boolean",
    "median-integral",
    "median-boolean",
)

# options (as mir keys)
KEYS = {
    "statistics": "interpolation-statistics",
    "k": "nclosest",
}


def _mir_spec(spec) -> str:
    """mir spec (YAML), from a type or spec (dict, YAML/JSON string), default linear"""
    spec = mir_options(spec, KEYS)

    type = spec.pop("type", spec.pop("interpolation", "linear"))
    type = ALIASES.get(type, type)
    if type not in METHODS:
        raise ValueError(
            f"Interpolation: unsupported type '{type}', "
            f"choices are: {', '.join(METHODS)}"
        )

    stats = spec.get("interpolation-statistics")
    if stats is not None and stats not in STATISTICS:
        raise ValueError(
            f"Interpolation: unsupported statistics '{stats}', "
            f"choices are: {', '.join(STATISTICS)}"
        )

    return to_yaml(dict(type=METHODS[type], **spec))


cdef class Interpolation:
    """Interpolation method (mir), from a type and options (spec)

    Types (and options): linear, bilinear, grid-box-average, grid-box-statistics
    (statistics), healpix-resample, healpix-statistics (statistics), nn, k-nn (k),
    voronoi-statistics (statistics), structured-bilinear, structured-bicubic and
    structured-biquasicubic; statistics are as STATISTICS. Its spec builds the same
    interpolation, as a type alone if built from it.
    """

    cdef const mir.Method* _method

    def __cinit__(self, spec=None, **kwargs):
        self._method = NULL

        if kwargs:
            assert not spec
            spec = kwargs
        elif spec is None:
            spec = dict()
        elif isinstance(spec, Interpolation):
            spec = spec.spec

        self._method = mir.MethodFactory.make_from_string(_mir_spec(spec))

    def __dealloc__(self):
        del self._method

    def __eq__(self, other) -> bool:
        if not isinstance(other, Interpolation):
            return NotImplemented
        return mir.method_equal(
            dereference(self._method), dereference((<Interpolation>other)._method)
        )

    @property
    def type(self) -> str:
        """Type (mir), as the spec"""
        return self._method.type()

    @property
    def json(self) -> str:
        """Spec (JSON), or the type alone if built from it"""
        return self._method.json_str()

    @property
    def spec(self):
        """Spec (dict), or the type alone if built from it (str)"""
        return from_json(self.json)

    def __repr__(self) -> str:
        return f"Interpolation({self.json})"

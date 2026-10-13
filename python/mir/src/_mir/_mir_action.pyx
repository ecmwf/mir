cimport mir_defs as mir
from cython.operator cimport dereference

from mir._spec import from_json
from mir._spec import mir_options
from mir._spec import to_yaml

# action types (as mir actions)
ACTIONS = {
    "crop": "crop.area",
    "mask": "mask.area",
    "bandpass": "filter.sh-bandpass",
    "limiter": "filter.limiter",
    "scalar-gradient": "filter.scalar-gradient",
    "scalar-laplacian": "filter.scalar-laplacian",
    "uv-gradient": "filter.uv-gradient",
    "uv-divergence": "filter.uv-divergence",
    "uv-vorticity": "filter.uv-vorticity",
}

ALIASES = {mir_type: type for type, mir_type in ACTIONS.items()}

# options (as mir keys)
KEYS = {
    "minimum": "limiter-minimum",
    "maximum": "limiter-maximum",
}


def _type_and_mir_spec(spec):
    """mir type and spec (YAML), from a type, or a spec (dict or YAML/JSON string)"""
    spec = mir_options(spec, KEYS)

    type = spec.pop("type", None)
    type = ALIASES.get(type, type)
    if type not in ACTIONS:
        raise ValueError(
            f"Action: unsupported type '{type}', choices are: {', '.join(ACTIONS)}"
        )

    if "area" in spec:
        spec["area"] = list(spec["area"])

    return ACTIONS[type], to_yaml(dict(type=ACTIONS[type], **spec))


cdef class Action:
    """Action (mir), from a type and options (spec)

    Types (and options): crop (area: [north, west, south, east]), mask (area),
    bandpass (bandpass: [min M, max M, min N, max N], truncation), limiter (minimum,
    maximum), scalar-gradient, scalar-laplacian, uv-gradient, uv-divergence and
    uv-vorticity. Its spec builds the same action, as a type alone if built from it.
    """

    cdef const mir.Action* _action
    cdef str _type

    def __cinit__(self, spec=None, **kwargs):
        self._action = NULL

        if kwargs:
            assert not spec
            spec = kwargs
        elif isinstance(spec, Action):
            spec = spec.spec

        self._type, mir_spec = _type_and_mir_spec(spec)
        self._action = mir.ActionFactory.make_from_string(mir_spec)

    def __dealloc__(self):
        del self._action

    def __eq__(self, other) -> bool:
        if not isinstance(other, Action):
            return NotImplemented
        return mir.action_equal(
            dereference(self._action), dereference((<Action>other)._action)
        )

    @property
    def type(self) -> str:
        """Type (mir), as the spec"""
        return self._type

    @property
    def json(self) -> str:
        """Spec (JSON, of mir types), or the (mir) type alone if built from it"""
        return self._action.json_str()

    @property
    def spec(self):
        """Spec (dict, of mir types), or the (mir) type alone if built from it (str)"""
        return from_json(self.json)

    def __repr__(self) -> str:
        return f"Action({self.json})"

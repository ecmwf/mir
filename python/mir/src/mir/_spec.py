# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

"""Specs: to mir as YAML (parsed), and from mir as JSON (or a name alone)"""

import json

import yaml


def to_yaml(spec: dict) -> str:
    """YAML (flow style, on one line), for mir"""
    return yaml.dump(spec, default_flow_style=True, width=float("inf")).strip()


def from_yaml(spec) -> dict:
    """Spec (dict), from a dict, a YAML/JSON mapping, or a type (name)"""
    if isinstance(spec, dict):
        return dict(spec)
    if isinstance(spec, str):
        return yaml.safe_load(spec) if spec.strip().startswith("{") else dict(type=spec)
    raise TypeError(f"unsupported spec type: {type(spec)}")


def mir_options(spec, keys: dict) -> dict:
    """Spec (dict) with mir keys (hyphens, or as keys), from a spec as from_yaml, without unset values"""
    return {
        keys.get(k.replace("_", "-"), k.replace("_", "-")): v
        for k, v in from_yaml(spec).items()
        if v is not None and not (isinstance(v, str) and not v.strip())
    }


def from_json(spec: str):
    """Spec (dict), or a name alone (str), from mir"""
    return json.loads(spec) if spec.strip().startswith("{") else spec

# SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

docker run -it --name mirpythonbuild -v .:/src/mir-python quay.io/pypa/manylinux_2_28_x86_64 /src/mir-python/build.sh

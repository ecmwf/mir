# Copyright 2005-2007 ECMWF
# 
# Licensed under the GNU Lesser General Public License which
# incorporates the terms and conditions of version 3 of the GNU
# General Public License.
# See LICENSE and gpl-3.0.txt for details.

# Usage: go_test.sh <PATH_TO_MARS_FOLDER>

set -ea
IS_DEBUG=0

client_loc=$1

unset ECREGRID_DEFINITIONS_PATH
unset ECREGRID_LSM_PATH
unset ECREGRID_LEGENDRE_COEFFICIENTS_PATH

unset MARS_HOME

export MARS_HOME=${client_loc}
echo "have set Mars home to $MARS_HOME"

export MARS_DEBUG=${IS_DEBUG}

# some ecregrid definitions that will be ignored 
# if not using ecregrid so no harm setting them

export ECREGRID_EMOS_SIMULATION=1
export ECREGRID_DEBUG=${IS_DEBUG}


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

unset MARS_HOME

export MARS_HOME=${client_loc}
echo "have set Mars home to $MARS_HOME"

export MARS_DEBUG=${IS_DEBUG}




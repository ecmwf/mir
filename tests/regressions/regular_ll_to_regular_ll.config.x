# input
regular_ll.2-2.grib2
# post-process (env. set by cmake; dry run, as execution fails: UserError: FailMethod: this interpolation method fails intentionally)
--grid=3/3 --area=custom
--dryrun
# action-plan
[{"type":"interpolate.grid2regular-ll","interpolation":"fail","grid":[3,3],"area":[89.5,0,-87.5,357]},{"type":"crop.area","area":[46,12,41.5,19]},{"type":"save"}]

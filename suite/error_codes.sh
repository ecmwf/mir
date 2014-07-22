#!/usr/bin/ksh

# Set up some error codes
TEST_PASS=0
TEST_FAIL=1
TEST_RESOURCE_MINOR=2
TEST_RESOURCE_MAJOR=3
TEST_FILE_INTEGRITY_FAIL=4
TEST_FILE_DIFFERENCE_MINOR=5
TEST_FILE_DIFFERENCE_MAJOR=6
TEST_WRONG_TARGET_GRID=7
TEST_RESOURCE_SUPERIOR=8

# Implement some descriptions
set -A codes
codes[$TEST_PASS]="Pass"
codes[$TEST_FAIL]="Test Failure"
codes[$TEST_RESOURCE_MINOR]="Performance(Minor)"
codes[$TEST_RESOURCE_MAJOR]="Performance(Major)"
codes[$TEST_FILE_INTEGRITY_FAIL]="File Structure"
codes[$TEST_FILE_DIFFERENCE_MINOR]="Results(Minor)"
codes[$TEST_FILE_DIFFERENCE_MAJOR]="Results(Major)"
codes[$TEST_WRONG_TARGET_GRID]="Wrong target grid"
codes[$TEST_RESOURCE_SUPERIOR]="Performance(Superior)"


#etc

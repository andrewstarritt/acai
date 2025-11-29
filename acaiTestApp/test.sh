#!/bin/bash
#

export PATH="../bin/${EPICS_HOST_ARCH:?}:${PATH}"

test_csnprintf  > z1
colordiff test_csnprintf.out  z1
rm -f z1
echo

test_client_set > z2
colordiff test_client_set.out z2
rm -f z2
echo

test_abstract_user > z3
colordiff  test_abstract_user.out  z3
rm -f z3
echo

# end

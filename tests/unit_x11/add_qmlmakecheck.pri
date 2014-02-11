# Do not use CONFIG += testcase that would add a 'make check' because it also
# adds a 'make install' that installs the test cases, which we do not want.
# Instead add a 'make check' manually.

check.target = check

# Xvfb doesn't run on armhf/qemu
!contains(QMAKE_HOST.arch,armv7l) {
 check.commands = "set -e;"
 for(TEST, TESTS) {
  check.commands += ../../unit/runtest.sh $${TARGET} $${TEST};
 }
}

# CMake generated Testfile for 
# Source directory: $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests
# Build directory: $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/traffic_gen_test_UBSAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/traffic_gen_test_ASAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/traffic_gen_test_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/adp_fuzz_test_UBSAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/adp_fuzz_test_ASAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/adp_fuzz_test_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/ptp_flags_test_UBSAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/ptp_flags_test_ASAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/tests/ptp_flags_test_e3b0c442_include.cmake")
add_test("corpus_validates" "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/head-unset-Debug/traffic-gen/packet_gen" "--yaml-dir" "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/protocols" "--stack-file" "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/stacks/aecp_acquire_entity.yaml" "--validate")
set_tests_properties("corpus_validates" PROPERTIES  _BACKTRACE_TRIPLES "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/CMakeLists.txt;73;add_test;$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/head/traffic-gen/tests/CMakeLists.txt;0;")

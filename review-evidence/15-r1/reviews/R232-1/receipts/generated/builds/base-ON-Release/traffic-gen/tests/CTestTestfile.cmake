# CMake generated Testfile for 
# Source directory: $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/traffic-gen/tests
# Build directory: $WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/traffic_gen_test_UBSAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/traffic_gen_test_ASAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/traffic_gen_test_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/adp_fuzz_test_UBSAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/adp_fuzz_test_ASAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/adp_fuzz_test_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/ptp_flags_test_UBSAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/ptp_flags_test_ASAN_e3b0c442_include.cmake")
include("$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/tests/ptp_flags_test_e3b0c442_include.cmake")
add_test("corpus_validates" "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/builds/base-ON-Release/traffic-gen/packet_gen" "--yaml-dir" "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/protocols" "--stack-file" "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/stacks/aecp_acquire_entity.yaml" "--validate")
set_tests_properties("corpus_validates" PROPERTIES  _BACKTRACE_TRIPLES "$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/traffic-gen/tests/CMakeLists.txt;73;add_test;$WORKSPACE_HOME/milan-fpga-management/2026-09-22/18-r1-r232/work/base/traffic-gen/tests/CMakeLists.txt;0;")

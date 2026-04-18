cmake_minimum_required(VERSION 3.31)
project(tsn_sim VERSION 0.0.1
    DESCRIPTION "ns-3 + Verilator co-simulation via SystemC")

##################### DEPENDENCIES #########################

# SystemC — requires a system install or SYSTEMC_ROOT hint.
find_package(SystemCLanguage CONFIG REQUIRED)

# ns-3 — requires ns-3 built with --enable-systemc.
# Adjust COMPONENTS to match the ns-3 modules your topology uses.
find_package(ns3 REQUIRED COMPONENTS
    core
    network
    internet
    applications
)

##################### SOURCES ##############################

set(SIM_INC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
    # tsn-gen headers required by VerilatorNetDevice and sc_main
    ${CMAKE_SOURCE_DIR}/traffic-gen/inc
    ${CMAKE_SOURCE_DIR}/parser/inc
    # Verilator-generated DUT headers (obj_dir produced by verilator --sc)
    ${CMAKE_SOURCE_DIR}/obj_dir
)

add_library(tsn_sim_lib STATIC
    src/axis_bridge.cpp
    src/verilator_net_device.cpp
)

target_include_directories(tsn_sim_lib PUBLIC
    ${SIM_INC_DIRS}
)

target_link_libraries(tsn_sim_lib PUBLIC
    SystemC::systemc
    ns3::libcore
    ns3::libnetwork
    ns3::libinternet
    ns3::libapplications
    traffic_gen
    protocol_parser
)

target_compile_features(tsn_sim_lib PUBLIC cxx_std_17)

##################### EXECUTABLE ###########################

# The Verilator-generated library is produced by `verilator --sc --build`.
# It lives in obj_dir/ relative to the project root.
add_executable(tsn_cosim
    sc_main.cpp
)

target_include_directories(tsn_cosim PRIVATE
    ${SIM_INC_DIRS}
)

target_link_libraries(tsn_cosim PRIVATE
    tsn_sim_lib
    # Verilator runtime and generated DUT — adjust library name to match your
    # top module (e.g. VTsnEndpoint → libVTsnEndpoint.a from obj_dir).
    ${CMAKE_SOURCE_DIR}/obj_dir/libVTsnEndpoint.a
    ${VERILATOR_ROOT}/include/verilated.cpp
)

##################### COMPILE OPTIONS ######################

target_compile_options(tsn_cosim PRIVATE
    $<$<CONFIG:Debug>:-O0 -g3>
    $<$<CONFIG:Release>:-O3 -g3>
)

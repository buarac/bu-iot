# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)

set(EXTRA_COMPONENT_DIRS 
    "../../components/bu_common"
    "../../components/bu_espnow"
    "../../components/bu_wifi"
)

set(PROJECT_VER "0.0.0.1")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options (-fdiagnostics-color=always)
project(bu_inject)
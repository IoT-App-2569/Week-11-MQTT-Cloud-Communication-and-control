cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# ระบุแค่โฟลเดอร์ที่มีอยู่จริง
list(APPEND EXTRA_COMPONENT_DIRS 
    "esp-rainmaker/components"
    "esp-rainmaker/examples/common/rmaker_app_network"
)

project(Lab11-4_RainMaker_Light)
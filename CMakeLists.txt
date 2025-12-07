cmake_minimum_required(VERSION 3.16)
project(HeatMapOverlay LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 允许显式指定 Qt 主版本，避免 Qt Creator 与 Qt 安装版本不一致时加载失败
# 默认使用 Qt5（匹配 Qt 5.15.2 Designer），只有显式传入 -DHEATMAP_QT_MAJOR=6 时才会选择 Qt6
set(HEATMAP_QT_MAJOR "5" CACHE STRING "Force Qt major version (5 or 6) to match host tools (Qt Creator/Designer)")
if(HEATMAP_QT_MAJOR STREQUAL "6")
    find_package(Qt6 COMPONENTS Widgets Gui Designer REQUIRED)
    set(QT_FOUND_VERSION 6)
    set(QT_PREFIX Qt6)
elseif(HEATMAP_QT_MAJOR STREQUAL "5")
    find_package(Qt5 5.15 COMPONENTS Widgets Gui Designer REQUIRED)
    set(QT_FOUND_VERSION 5)
    set(QT_PREFIX Qt5)
else()
    message(FATAL_ERROR "Unknown HEATMAP_QT_MAJOR='${HEATMAP_QT_MAJOR}'. Please set to 5 (Qt 5.15.2 Designer) or 6 (Qt Creator 6).")
endif()

message(STATUS "HeatMapOverlay using Qt${QT_FOUND_VERSION} (${QT_PREFIX})")

set(HEATMAP_SOURCES
    src/HeatMapOverlay.cpp
)

set(HEATMAP_HEADERS
    src/HeatMapOverlay.h
)

add_library(heatmapoverlay STATIC ${HEATMAP_SOURCES} ${HEATMAP_HEADERS})
target_link_libraries(heatmapoverlay PRIVATE ${QT_PREFIX}::Widgets ${QT_PREFIX}::Gui)

# 设计师插件，方便直接添加到 Qt 控件库
add_library(HeatMapOverlayPlugin SHARED
    plugin/HeatMapOverlayPlugin.cpp
    plugin/HeatMapOverlayPlugin.h
)

# 插件需要能够找到实际控件的头文件


target_include_directories(HeatMapOverlayPlugin PRIVATE src)
target_link_libraries(HeatMapOverlayPlugin PRIVATE heatmapoverlay ${QT_PREFIX}::Designer)
set_target_properties(HeatMapOverlayPlugin PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/designer"
)

# Demo 应用
add_executable(heatmap_demo
    demo/main.cpp
)

target_include_directories(heatmap_demo PRIVATE src)
target_link_libraries(heatmap_demo PRIVATE heatmapoverlay ${QT_PREFIX}::Widgets)

install(TARGETS heatmapoverlay HeatMapOverlayPlugin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib/designer
        RUNTIME DESTINATION bin)

install(FILES ${HEATMAP_HEADERS} DESTINATION include/heatmapoverlay)

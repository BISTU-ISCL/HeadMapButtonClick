cmake_minimum_required(VERSION 3.16)
project(HeatMapOverlay LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 优先尝试 Qt6，其次 Qt5，便于在不同环境构建
find_package(Qt6 COMPONENTS Widgets Gui Designer QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Widgets Gui Designer REQUIRED)
    set(QT_FOUND_VERSION 5)
    set(QT_PREFIX Qt5)
else()
    set(QT_FOUND_VERSION 6)
    set(QT_PREFIX Qt6)
endif()

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

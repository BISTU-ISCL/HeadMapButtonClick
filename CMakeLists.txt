cmake_minimum_required(VERSION 3.16)
project(HeatMapOverlay LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows 环境固定使用 Qt 6.10.0（MSVC 2022 64-bit Release），以与 Qt Creator 18.0.0 与 Qt Designer 保持一致。
find_package(Qt6 6.10.0 EXACT COMPONENTS Widgets Gui Designer REQUIRED)
set(QT_PREFIX Qt6)
message(STATUS "HeatMapOverlay using Qt6.10.0 (${QT_PREFIX}) for MSVC 2022 64-bit")

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
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/designer"
)

# Demo 应用
add_executable(heatmap_demo
    demo/main.cpp
)

target_include_directories(heatmap_demo PRIVATE src)
target_link_libraries(heatmap_demo PRIVATE heatmapoverlay ${QT_PREFIX}::Widgets)

install(TARGETS heatmapoverlay HeatMapOverlayPlugin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION plugins/designer
        RUNTIME DESTINATION plugins/designer)

install(FILES ${HEATMAP_HEADERS} DESTINATION include/heatmapoverlay)

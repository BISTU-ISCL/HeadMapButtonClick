cmake_minimum_required(VERSION 3.16)
project(HeatMapOverlay LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 仅支持 Qt 5.15.2，保证与指定开发环境完全一致，避免 Qt Creator 6 (Qt6) 等其他版本无法加载。
find_package(Qt5 5.15.2 EXACT COMPONENTS Widgets Gui Designer REQUIRED)
set(QT_PREFIX Qt5)
message(STATUS "HeatMapOverlay using Qt5.15.2 (${QT_PREFIX})")

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

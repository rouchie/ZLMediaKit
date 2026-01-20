
# 查找 Qt5 包
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(spdlog REQUIRED)

# 设置自动处理 moc、uic、rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 添加可执行文件
add_executable(rtsp_client
    main.cpp

    MainWindow.cpp
    MainWindow.h
    RtspView.cpp
    RtspView.h
)

# 链接 Qt 库
target_link_libraries(rtsp_client ${PROJECT_NAME} Qt5::Widgets spdlog::spdlog)

# 在 Windows 上设置子系统
if(WIN32)
    set_target_properties(rtsp_client PROPERTIES WIN32_EXECUTABLE TRUE)
endif()


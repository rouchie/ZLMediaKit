# 主应用程序配置
add_executable(app app/main.cpp)
target_link_libraries(app PRIVATE ${PROJECT_NAME})

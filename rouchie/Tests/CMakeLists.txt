# 查找 GTest
find_package(GTest REQUIRED)

# 获取所有测试源文件
aux_source_directory(. TEST_SOURCES)

# 为每个测试源文件创建独立可执行文件
foreach(test_source ${TEST_SOURCES})
    # 获取文件名（不带扩展名）
    get_filename_component(exe_name ${test_source} NAME_WE)
    
    # 创建可执行文件
    add_executable(${exe_name} ${test_source})
    
    # 链接必要的库
    target_link_libraries(${exe_name} PRIVATE 
		GTest::gtest
		GTest::gtest_main
        ${PROJECT_NAME}
    )
    
    # 注册到 CTest
    gtest_discover_tests(${exe_name})
    
    # 将测试可执行文件添加到列表（可选，用于批量操作）
    list(APPEND ALL_TEST_EXECUTABLES ${exe_name})
    
    message(STATUS "add ${exe_name} source(${test_source})")
endforeach()

# 可选：创建一个总测试目标，运行所有测试
add_custom_target(run_all_tests
    COMMAND ctest --output-on-failure
    DEPENDS ${ALL_TEST_EXECUTABLES}
    COMMENT "运行所有测试"
)
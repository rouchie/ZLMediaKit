cmake_minimum_required(VERSION 3.29)
project(Rouchie)

find_package(spdlog REQUIRED)

set(Rouchie_SRC_LIST
	Module/MP4ReaderMod.cpp
	Module/MP4ReaderMod.h
	Module/RtspPlayerMod.cpp
	Module/RtspPlayerMod.h

	Base/RQCodeID.h
	Base/RQCommandID.h
	Base/RQCore.cpp
	Base/RQCore.h
	Base/RQDefines.h
	Base/RQModuleBase.cpp
	Base/RQModuleBase.h
	Base/RQModuleID.h
	Base/RQMsg.cpp
	Base/RQMsg.h
	Base/RQTimer.cpp
	Base/RQTimer.h
)

add_library(${PROJECT_NAME} STATIC ${Rouchie_SRC_LIST})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}
)

set(COMPILE_DEFINITIONS ${MK_COMPILE_DEFINITIONS})

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMPILE_DEFINITIONS})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTIONS_DEFAULT})

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_link_libraries(${PROJECT_NAME} -Wl,--start-group ${MK_LINK_LIBRARIES} -Wl,--end-group)
else()
  target_link_libraries(${PROJECT_NAME} PUBLIC ${MK_LINK_LIBRARIES} spdlog::spdlog)
endif()

if(MSVC)
  set(RESOURCE_FILE "${CMAKE_SOURCE_DIR}/resource.rc")
  set_source_files_properties(${RESOURCE_FILE} PROPERTIES LANGUAGE RC)  
  target_sources(Rouchie PRIVATE ${RESOURCE_FILE})
endif()

add_executable(app app/main.cpp)
target_link_libraries(app PRIVATE ${PROJECT_NAME})

add_subdirectory(Tests)
add_subdirectory(rtspclient)

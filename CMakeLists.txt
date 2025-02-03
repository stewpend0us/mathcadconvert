cmake_minimum_required(VERSION 3.14)

project(mathcadconvert)

include(FetchContent)

set(PUGIXML_NO_XPATH ON)
FetchContent_Declare(
	fetch_pugixml
	GIT_REPOSITORY https://github.com/zeux/pugixml.git
	GIT_TAG "v1.15"
	GIT_SHALLOW TRUE
	GIT_PROGRESS ON
)

set(CATCH_INSTALL_DOCS OFF)
set(CATCH_INSTALL_EXTRAS OFF)
FetchContent_Declare(
	fetch_Catch2
	GIT_REPOSITORY https://github.com/catchorg/Catch2.git
	GIT_TAG "v3.8.0"
	GIT_SHALLOW TRUE
	GIT_PROGRESS ON
)

FetchContent_MakeAvailable(fetch_pugixml fetch_Catch2)


add_executable(mathcadconvert src/main.cpp src/matlab.cpp)
target_compile_features(mathcadconvert PUBLIC c_std_99 cxx_std_23)
target_link_libraries(mathcadconvert pugixml)
target_include_directories(mathcadconvert PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>"
	"$<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>"
)

add_executable(test_simple_tags test/simple_tags.cpp src/matlab.cpp)
target_compile_features(test_simple_tags PUBLIC c_std_99 cxx_std_23)
target_link_libraries(test_simple_tags pugixml Catch2WithMain)
target_include_directories(test_simple_tags PUBLIC
	"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>"
	"$<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${CMAKE_INSTALL_INCLUDEDIR}>"
)


#list(APPEND CATCH_WARNING_TARGETS test_simple_tags)
#set(CATCH_WARNING_TARGETS ${CATCH_WARNING_TARGETS} PARENT_SCOPE)
cmake_minimum_required(VERSION 3.22)
project(idlgen)

include(env.cmake)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(CLANG_CONFIG_TYPE "Debug")
elseif (${CMAKE_BUILD_TYPE} STREQUAL "Release")
	set(CLANG_CONFIG_TYPE "Release")
else()
	message(FATAL_ERROR "Unknown build type ${CMAKE_BUILD_TYPE}")
endif()

set(CLANG_BUILD_OUT_DIR "${CLANG_BUILD_OUT_ROOT_DIR}/${CLANG_CONFIG_TYPE}")

set(CLANG_LIB_DIR "${CLANG_BUILD_OUT_DIR}/lib")
set(CLANG_INCLUDE_DIR "${LLVM_PROJECT_SRC_DIR}/clang/include")
set(CLANG_GEN_INCLUDE_DIR "${CLANG_BUILD_OUT_DIR}/tools/clang/include")
set(LLVM_INCLUDE_DIR "${LLVM_PROJECT_SRC_DIR}/llvm/include")
set(LLVM_GEN_INCLUDE_DIR "${CLANG_BUILD_OUT_DIR}/include")
message(CLANG_LIB_DIR=${CLANG_LIB_DIR})
message(CLANG_INCLUDE_DIR=${CLANG_INCLUDE_DIR})
message(CLANG_GEN_INCLUDE_DIR=${CLANG_GEN_INCLUDE_DIR})
message(LLVM_INCLUDE_DIR=${LLVM_INCLUDE_DIR})
message(LLVM_GEN_INCLUDE_DIR=${LLVM_GEN_INCLUDE_DIR})

file(GLOB LIBS CONFIGURE_DEPENDS "${CLANG_LIB_DIR}/*.lib")

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS "src/*.cpp"
			  CONFIGURE_DEPENDS "src/*.h")
add_executable(idlgen GenerateIdl.cpp ${SRC})

target_include_directories(idlgen PRIVATE "src" ${CLANG_INCLUDE_DIR} ${CLANG_GEN_INCLUDE_DIR} ${LLVM_INCLUDE_DIR} ${LLVM_GEN_INCLUDE_DIR})
add_compile_definitions(UNICODE)
set_target_properties(idlgen PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED ON
		CMAKE_CXX_STANDARD 17
		CMAKE_CXX_STANDARD_REQUIRED ON)
target_link_libraries(idlgen "version.lib")
MATH(EXPR LIB_COUNT "0")
foreach(file ${LIBS})
	set(LIB_NAME "idlgen_${LIB_COUNT}")
	add_library(${LIB_NAME} STATIC IMPORTED)
	set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION ${file})
	target_link_libraries(idlgen ${LIB_NAME})
	MATH(EXPR LIB_COUNT "${LIB_COUNT} + 1")
endforeach()
message("-- Linked ${LIB_COUNT} libs for idlgen")
cmake_minimum_required(VERSION 3.12)
project(qubic-cli CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# keep the lists sorted alphabetically
SET(FILES 	${CMAKE_SOURCE_DIR}/asset_utils.cpp
		${CMAKE_SOURCE_DIR}/connection.cpp
		${CMAKE_SOURCE_DIR}/contracts.cpp
		${CMAKE_SOURCE_DIR}/file_upload.cpp
		${CMAKE_SOURCE_DIR}/key_utils.cpp
		${CMAKE_SOURCE_DIR}/main.cpp
		${CMAKE_SOURCE_DIR}/msvault.cpp
		${CMAKE_SOURCE_DIR}/node_utils.cpp
		${CMAKE_SOURCE_DIR}/nostromo.cpp
		${CMAKE_SOURCE_DIR}/oracle_utils.cpp
		${CMAKE_SOURCE_DIR}/proposal.cpp
		${CMAKE_SOURCE_DIR}/qbond.cpp
		${CMAKE_SOURCE_DIR}/qearn.cpp
		${CMAKE_SOURCE_DIR}/qpi_adapter.cpp
		${CMAKE_SOURCE_DIR}/qrwa.cpp
		${CMAKE_SOURCE_DIR}/qswap.cpp
		${CMAKE_SOURCE_DIR}/quottery.cpp
		${CMAKE_SOURCE_DIR}/qutil.cpp
		${CMAKE_SOURCE_DIR}/qvault.cpp
		${CMAKE_SOURCE_DIR}/qx.cpp
		${CMAKE_SOURCE_DIR}/escrow.cpp
		${CMAKE_SOURCE_DIR}/sc_utils.cpp
		${CMAKE_SOURCE_DIR}/test_utils.cpp
		${CMAKE_SOURCE_DIR}/wallet_utils.cpp
		${CMAKE_SOURCE_DIR}/utils.cpp
)
SET(HEADER_FILES
	argparser.h	
	asset_utils.h
	common_functions.h
	connection.h
	contracts.h
	defines.h
	fourq_qubic.h
	global.h
	k12_and_key_utils.h
	key_utils.h
	logger.h
	msvault.h
	node_utils.h
	nostromo.h
	qpi_adapter.h
	oracle_utils.h
	prompt.h
	proposal.h
	qrwa.h
	qbond.h
	qearn.h
	qswap.h
	qswap_struct.h
	quottery.h
	qutil.h
	qvault.h
	qx.h
	escrow.h
	qx_struct.h
	sanity_check.h
	sc_utils.h
	structs.h
	test_utils.h
	utils.h
	wallet_utils.h
)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

ADD_EXECUTABLE(qubic-cli main.cpp ${FILES} ${HEADER_FILES})
set_property(TARGET qubic-cli PROPERTY COMPILE_WARNING_AS_ERROR ON)
target_include_directories(qubic-cli PUBLIC ${CMAKE_SOURCE_DIR}/submodules)
target_include_directories(qubic-cli PUBLIC ${CMAKE_SOURCE_DIR}/submodules/core)
target_include_directories(qubic-cli PUBLIC ${CMAKE_SOURCE_DIR}/submodules/core/src)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	if(APPLE)
		add_compile_definitions(QUBIC_ARCH_ARM64=1)
		target_include_directories(qubic-cli BEFORE PRIVATE
				${CMAKE_SOURCE_DIR}/arm_compat
		)
	else()
		target_compile_options(${PROJECT_NAME} PRIVATE -mrdrnd -mavx2)
	endif()
endif()

# to make clang++ shut up about "first argument in call to 'memcpy' is a pointer to non-trivially copyable type 'id'"
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(${PROJECT_NAME} PRIVATE -Wno-nontrivial-memaccess)
endif()

ADD_LIBRARY(fourq-qubic SHARED fourq_qubic.cpp)
set_property(TARGET fourq-qubic PROPERTY SOVERSION 1)
target_compile_options(fourq-qubic PRIVATE -DBUILD_4Q_LIB)
install(TARGETS fourq-qubic LIBRARY)
install(TARGETS qubic-cli RUNTIME)


# Opt-in unit tests (off by default, so normal and CI builds are unchanged):
#   cmake -DQUBIC_CLI_BUILD_TESTS=ON -S . -B build
#   cmake --build build --target qubic-cli-tests && ctest --test-dir build
option(QUBIC_CLI_BUILD_TESTS "Build qubic-cli unit tests" OFF)
if(QUBIC_CLI_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_executable(qubic-cli-tests tests/sanity_check_test.cpp)
    target_include_directories(qubic-cli-tests PRIVATE ${CMAKE_SOURCE_DIR})
    target_link_libraries(qubic-cli-tests PRIVATE GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(qubic-cli-tests)
endif()

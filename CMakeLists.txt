cmake_minimum_required(VERSION 3.12)
project(qubic-cli CXX)
set (CMAKE_CXX_STANDARD 17)

# keep the lists sorted alphabetically
SET(FILES 	${CMAKE_SOURCE_DIR}/asset_utils.cpp
		${CMAKE_SOURCE_DIR}/connection.cpp
		${CMAKE_SOURCE_DIR}/file_upload.cpp
		${CMAKE_SOURCE_DIR}/key_utils.cpp
		${CMAKE_SOURCE_DIR}/main.cpp
		${CMAKE_SOURCE_DIR}/msvault.cpp
		${CMAKE_SOURCE_DIR}/node_utils.cpp
		${CMAKE_SOURCE_DIR}/nostromo.cpp
		${CMAKE_SOURCE_DIR}/proposal.cpp
		${CMAKE_SOURCE_DIR}/qbond.cpp
		${CMAKE_SOURCE_DIR}/qearn.cpp
		${CMAKE_SOURCE_DIR}/qswap.cpp
		${CMAKE_SOURCE_DIR}/quottery.cpp
		${CMAKE_SOURCE_DIR}/qutil.cpp
		${CMAKE_SOURCE_DIR}/qvault.cpp
		${CMAKE_SOURCE_DIR}/qx.cpp
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
	defines.h
	fourq_qubic.h
	global.h
	k12_and_key_utils.h
	key_utils.h
	logger.h
	msvault.h
	node_utils.h
	nostromo.h
	prompt.h
	proposal.h
	qbond.h
	qearn.h
	qswap.h
	qswap_struct.h
	quottery.h
	qutil.h
	qvault.h
	qx.h
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
ADD_LIBRARY(fourq-qubic SHARED fourq_qubic.cpp)
set_property(TARGET fourq-qubic PROPERTY SOVERSION 1)
target_compile_options(fourq-qubic PRIVATE -DBUILD_4Q_LIB)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	target_compile_options(qubic-cli PRIVATE -mrdrnd)
	target_compile_options(fourq-qubic PRIVATE -mrdrnd)
endif()

install(TARGETS fourq-qubic LIBRARY)


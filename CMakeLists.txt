cmake_minimum_required(VERSION 3.12)
project(qubic-cli CXX)
set (CMAKE_CXX_STANDARD 17)
SET(FILES ${CMAKE_SOURCE_DIR}/connection.cpp
		  ${CMAKE_SOURCE_DIR}/keyUtils.cpp
		  ${CMAKE_SOURCE_DIR}/nodeUtils.cpp
		  ${CMAKE_SOURCE_DIR}/fileUpload.cpp
		  ${CMAKE_SOURCE_DIR}/walletUtils.cpp
		  ${CMAKE_SOURCE_DIR}/assetUtils.cpp
		  ${CMAKE_SOURCE_DIR}/SCUtils.cpp
		  ${CMAKE_SOURCE_DIR}/testUtils.cpp
		  ${CMAKE_SOURCE_DIR}/quottery.cpp
		  ${CMAKE_SOURCE_DIR}/qutil.cpp
		  ${CMAKE_SOURCE_DIR}/qx.cpp
		  ${CMAKE_SOURCE_DIR}/proposal.cpp
		  ${CMAKE_SOURCE_DIR}/qearn.cpp
		  ${CMAKE_SOURCE_DIR}/qvault.cpp
		  ${CMAKE_SOURCE_DIR}/msvault.cpp
)
SET(HEADER_FILES
	K12AndKeyUtil.h
	SCUtils.h
	argparser.h
	assetUtil.h
	connection.h
	defines.h
	fourq-qubic.h
	global.h
	keyUtils.h
	logger.h
	nodeUtils.h
	prompt.h
	quottery.h
	qutil.h
	sanityCheck.h
	structs.h
	utils.h
	walletUtils.h
	testUtils.h
	proposal.h
	qx.h
	qearn.h
	qvault.h
	msvault.h
)
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
ADD_EXECUTABLE(qubic-cli main.cpp ${FILES} ${HEADER_FILES})
set_property(TARGET qubic-cli PROPERTY COMPILE_WARNING_AS_ERROR ON)
ADD_LIBRARY(fourq-qubic SHARED fourq-qubic.cpp)
set_property(TARGET fourq-qubic PROPERTY SOVERSION 1)
target_compile_options(fourq-qubic PRIVATE -DBUILD_4Q_LIB)
install(TARGETS fourq-qubic LIBRARY)


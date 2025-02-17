cmake_minimum_required(VERSION 3.1)
project(qubic-cli C CXX)
set (CMAKE_CXX_STANDARD 11)
SET(FILES ${CMAKE_SOURCE_DIR}/connection.cpp
		  ${CMAKE_SOURCE_DIR}/keyUtils.cpp
		  ${CMAKE_SOURCE_DIR}/nodeUtils.cpp
		  ${CMAKE_SOURCE_DIR}/fileUpload.cpp
		  ${CMAKE_SOURCE_DIR}/walletUtils.cpp
		  ${CMAKE_SOURCE_DIR}/assetUtils.cpp
		  ${CMAKE_SOURCE_DIR}/SCUtils.cpp
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
	proposal.h
	qx.h
	qearn.h
	qvault.h
	msvault.h
)
ADD_EXECUTABLE(qubic-cli main.cpp ${FILES} ${HEADER_FILES})
ADD_LIBRARY(fourq-qubic SHARED fourq-qubic.cpp)
set_property(TARGET fourq-qubic PROPERTY SOVERSION 1)
target_compile_options(fourq-qubic PRIVATE -DBUILD_4Q_LIB)
install(TARGETS fourq-qubic LIBRARY)


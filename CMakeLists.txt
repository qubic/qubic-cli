cmake_minimum_required(VERSION 3.1)
project(qubic-cli C CXX)
find_package(OpenMP)
set (CMAKE_CXX_STANDARD 11)
SET(FILES ${CMAKE_SOURCE_DIR}/connection.cpp
		  ${CMAKE_SOURCE_DIR}/keyUtils.cpp
		  ${CMAKE_SOURCE_DIR}/nodeUtils.cpp
		  ${CMAKE_SOURCE_DIR}/walletUtils.cpp
		  ${CMAKE_SOURCE_DIR}/assetUtils.cpp
		${CMAKE_SOURCE_DIR}/qubicLogParser.cpp
)
ADD_EXECUTABLE(qubic-cli main.cpp ${FILES})
target_link_libraries(qubic-cli OpenMP::OpenMP_CXX)

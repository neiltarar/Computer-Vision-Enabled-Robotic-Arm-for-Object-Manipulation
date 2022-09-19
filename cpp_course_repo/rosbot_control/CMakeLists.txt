cmake_minimum_required(VERSION 3.0.2)
project(rosbot_control)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  geometry_msgs
  std_msgs
  nav_msgs
  sensor_msgs
)

catkin_package(
	INCLUDE_DIRS include
	LIBRARIES ${PROJECT_NAME}
	CATKIN_DEPENDS roscpp
#  DEPENDS system_lib
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

## Declare a C++ library
add_library(${PROJECT_NAME}
	src/rosbot_class.cpp
	)
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES})


set(name rosbot_class)
add_executable(${name} src/${name}.cpp)
add_dependencies(${name} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${name} ${catkin_LIBRARIES})

install(TARGETS ${PROJECT_NAME}
	ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
	RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
)

## Mark cpp header files for installation
install(DIRECTORY include/${PROJECT_NAME}/
	DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
)
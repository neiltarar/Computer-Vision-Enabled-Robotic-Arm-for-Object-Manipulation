cmake_minimum_required(VERSION 3.0.2)
project(c_scripts)

## Compile as C++11, supported in ROS Kinetic and newer
add_compile_options(-std=c++11)

find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  rosbot_control
  roscpp
  std_msgs
)

catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES c_scripts
#  CATKIN_DEPENDS geometry_msgs rosbot_control roscpp std_msgs
#  DEPENDS system_lib
)


include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)


set(csname unit1_exercise)
add_executable(${csname} src/${csname}.cpp)
add_dependencies(${csname} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${csname} ${catkin_LIBRARIES})

set(csname unit2_exercise)
add_executable(${csname} src/${csname}.cpp)
add_dependencies(${csname} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${csname} ${catkin_LIBRARIES})

set(csname unit3_exercise)
add_executable(${csname} src/${csname}.cpp)
add_dependencies(${csname} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${csname} ${catkin_LIBRARIES})

set(csname unit4_exercise)
add_executable(${csname} src/${csname}.cpp)
add_dependencies(${csname} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${csname} ${catkin_LIBRARIES})

set(csname unit5_exercise)
add_executable(${csname} src/${csname}.cpp)
add_dependencies(${csname} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${csname} ${catkin_LIBRARIES})

set(csname unit6_exercise)
add_executable(${csname} src/${csname}.cpp)
add_dependencies(${csname} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
target_link_libraries(${csname} ${catkin_LIBRARIES})
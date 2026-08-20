cmake_minimum_required(VERSION 3.8)
project(test_recovery_selector)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(behaviortree_cpp REQUIRED)
# find_package(recovery_selector REQUIRED)
find_package(nrg_behaviors REQUIRED)
find_package(ament_index_cpp REQUIRED)

# Create executables
add_executable(test_recovery_selector_bt src/test_recovery_selector_node.cpp)

# Add include directories
target_include_directories(test_recovery_selector_bt PUBLIC
  include
)

# Link dependencies to test_recovery_selector
ament_target_dependencies(test_recovery_selector_bt
  rclcpp
  behaviortree_cpp
  # recovery_selector
  nrg_behaviors
)

# Install behavior_trees directory
install(DIRECTORY behavior_trees
  DESTINATION share/${PROJECT_NAME}
)

# Install test_recovery_selector_bt
install(TARGETS test_recovery_selector_bt
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # The following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # The following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

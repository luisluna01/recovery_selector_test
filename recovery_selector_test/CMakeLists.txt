cmake_minimum_required(VERSION 3.8)
project(recovery_selector_test)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(recovery_selector REQUIRED)
find_package(nrg_behaviors REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(recovery_selector_test_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(magic_enum REQUIRED)
find_package(nrg_behaviors REQUIRED)

# Behaviors shared by the behavior tree executables
add_library(recovery_selector_test_behaviors STATIC
  src/behaviors/create_dummy_failure.cpp
  src/behaviors/dummy_task.cpp
  src/behaviors/simple_dummy_task.cpp
  src/behaviors/simple_request_user_intervention.cpp
  src/behaviors/dummy_recovery_strategy.cpp
  src/behaviors/dummy_condition.cpp
  src/behaviors/dummy_graceful_shutdown.cpp
  src/behaviors/no_failures_present.cpp
)
target_include_directories(recovery_selector_test_behaviors PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(recovery_selector_test_behaviors PUBLIC
  rclcpp
  behaviortree_cpp
  recovery_selector_test_msgs
  std_msgs
  std_srvs
)
target_link_libraries(recovery_selector_test_behaviors PUBLIC
  magic_enum::magic_enum
)
set_target_properties(recovery_selector_test_behaviors PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

# Create behavior tree node executables
add_executable(recovery_selector_test_bt src/recovery_selector_test_bt.cpp)
add_executable(example_traditional_bt src/example_traditional_bt.cpp)
add_executable(example_recovery_selector_bt src/example_recovery_selector_bt.cpp)

# Create dummy_failure_publisher node
add_executable(dummy_failure_publisher src/dummy_failure_publisher.cpp)
target_include_directories(dummy_failure_publisher PRIVATE
  include
)

# Create simple_dummy_failure_publisher node
add_executable(simple_dummy_failure_publisher src/simple_dummy_failure_publisher.cpp)
target_include_directories(simple_dummy_failure_publisher PRIVATE
  include
)

# Link shared behaviors (and their dependencies) to the behavior tree executables
target_link_libraries(recovery_selector_test_bt PRIVATE
  recovery_selector_test_behaviors
)
target_link_libraries(example_traditional_bt PRIVATE
  recovery_selector_test_behaviors
)
target_link_libraries(example_recovery_selector_bt PRIVATE
  recovery_selector_test_behaviors
)

# Link dependencies specific to each behavior tree executable
ament_target_dependencies(recovery_selector_test_bt PUBLIC
  ament_index_cpp
  recovery_selector
  nrg_behaviors
)
ament_target_dependencies(example_traditional_bt PUBLIC
  ament_index_cpp
  recovery_selector
  nrg_behaviors
)
ament_target_dependencies(example_recovery_selector_bt PUBLIC
  ament_index_cpp
  recovery_selector
  nrg_behaviors
)

# Link dependencies to dummy_failure_publisher node
ament_target_dependencies(dummy_failure_publisher
  rclcpp
  recovery_selector_test_msgs
)
target_link_libraries(dummy_failure_publisher
  magic_enum::magic_enum
)

# Link dependencies to simple_dummy_failure_publisher node
ament_target_dependencies(simple_dummy_failure_publisher
  rclcpp
  std_msgs
)

# Install executables
install(TARGETS recovery_selector_test_bt example_traditional_bt example_recovery_selector_bt
  dummy_failure_publisher
  simple_dummy_failure_publisher
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(
  DIRECTORY include/
  DESTINATION include
)

# Install behavior_trees, config and launch directory
install(
  DIRECTORY
    behavior_trees
    config
    launch
  DESTINATION share/${PROJECT_NAME}
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

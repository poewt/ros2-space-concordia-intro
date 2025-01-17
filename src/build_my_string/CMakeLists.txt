cmake_minimum_required(VERSION 3.8)
project(build_my_string)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclpy REQUIRED)
find_package(rclcpp REQUIRED)

# find dependencies
add_executable(publisher src/cpp_publisher.cpp)
ament_target_dependencies(publisher rclcpp)

# For custom types and interfaces
find_package(rosidl_default_generators REQUIRED)
set(msg_files
  "msg/Message.msg"
)
rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
)
ament_export_dependencies(rosidl_default_runtime)

# Link against the interface
rosidl_get_typesupport_target(cpp_typesupport_target
  ${PROJECT_NAME} rosidl_typesupport_cpp)

target_link_libraries(publisher "${cpp_typesupport_target}")

# this doesn't seem to work
#  ament_python_install_package(${PROJECT_NAME})

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

# install executables
install(TARGETS
  publisher
  DESTINATION lib/${PROJECT_NAME}
)

# install scripts
install(PROGRAMS
  scripts/py_subscriber.py
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()

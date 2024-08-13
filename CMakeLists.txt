cmake_minimum_required(VERSION 3.16)
project(teknic_hardware LANGUAGES CXX)

if(CMAKE_CXX_COMPILER_ID MATCHES "(GNU|Clang)")
  add_compile_options(-Wall -Wextra)
endif()

# find dependencies
set(THIS_PACKAGE_INCLUDE_DEPENDS
  hardware_interface
  pluginlib
  rclcpp
  rclcpp_lifecycle
  # Threads
)
find_package(ament_cmake REQUIRED)
foreach(Dependency IN ITEMS ${THIS_PACKAGE_INCLUDE_DEPENDS})
  find_package(${Dependency} REQUIRED)
endforeach()

# get build platform
if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL x86_64)
  set(HOST_PLATFORM x64)
elseif (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL aarch64)
  set(HOST_PLATFORM arm64)
else ()
  message(FATAL_ERROR "Host platform not supported." )
endif ()

## COMPILE
add_library(
  teknic_hardware
  SHARED
  src/system.cpp
)
target_compile_features(teknic_hardware PUBLIC cxx_std_17)
target_include_directories(teknic_hardware PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/teknic_hardware>)
ament_target_dependencies(
  teknic_hardware PUBLIC
  ${THIS_PACKAGE_INCLUDE_DEPENDS}
)
target_link_libraries(teknic_hardware PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/${HOST_PLATFORM}/libsFoundation20.so)

# Causes the visibility macros to use dllexport rather than dllimport,
# which is appropriate when building the dll but not consuming it.
target_compile_definitions(${PROJECT_NAME} PUBLIC "TEKNIC_HARDWARE_BUILDING_DLL")

# Export hardware plugins
pluginlib_export_plugin_description_file(hardware_interface teknic_hardware.xml)

# INSTALL
install(
  DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)
install(
  DIRECTORY lib/${HOST_PLATFORM}/
  DESTINATION lib
)
install(TARGETS teknic_hardware
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

## EXPORTS
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})
ament_package()

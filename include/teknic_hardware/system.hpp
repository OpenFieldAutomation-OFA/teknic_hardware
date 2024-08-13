// credit: https://github.com/ros-controls/ros2_control_demos
#ifndef TEKNIC_HARDWARE__SYSTEM_HPP_
#define TEKNIC_HARDWARE__SYSTEM_HPP_

#include <memory>
#include <string>
#include <vector>

#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "rclcpp/macros.hpp"
#include "rclcpp_lifecycle/node_interfaces/lifecycle_node_interface.hpp"
#include "rclcpp_lifecycle/state.hpp"
#include "teknic_hardware/visibility_control.h"
#include "sFoundation/pubSysCls.h"

namespace teknic_hardware
{
class TeknicSystemHardware : public hardware_interface::SystemInterface
{
public:
  RCLCPP_SHARED_PTR_DEFINITIONS(TeknicSystemHardware);
  
  virtual ~TeknicSystemHardware();

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_init(
    const hardware_interface::HardwareInfo & info) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_configure(
    const rclcpp_lifecycle::State & previous_state) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_cleanup(
    const rclcpp_lifecycle::State & previous_state) override;

  TEKNIC_HARDWARE_PUBLIC
  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  TEKNIC_HARDWARE_PUBLIC
  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::return_type prepare_command_mode_switch(
    const std::vector<std::string> & start_interfaces,
    const std::vector<std::string> & stop_interfaces) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::return_type perform_command_mode_switch(
    const std::vector<std::string> & start_interfaces,
    const std::vector<std::string> & stop_interfaces) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_activate(
    const rclcpp_lifecycle::State & previous_state) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::CallbackReturn on_deactivate(
    const rclcpp_lifecycle::State & previous_state) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::return_type read(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

  TEKNIC_HARDWARE_PUBLIC
  hardware_interface::return_type write(
    const rclcpp::Time & time, const rclcpp::Duration & period) override;

private:
  std::vector<double> hw_commands_positions_;
  std::vector<double> hw_commands_velocities_;
  std::vector<double> hw_states_positions_;
  std::vector<double> hw_states_velocities_;
  std::vector<double> hw_states_efforts_;

  std::vector<double> counts_conversions_;
  std::vector<int> homing_;
  std::vector<double> peak_torques_;
  std::vector<double> feed_constants_;
  std::vector<bool> read_only_;

  double count = 0;

  sFnd::SysManager* myMgr = sFnd::SysManager::Instance();
  std::vector<std::string> chports;
  std::vector<std::pair<std::size_t, std::size_t>> nodes;

  enum control_mode_t
  {
    SPEED_LOOP,
    POSITION_LOOP,
    UNDEFINED
  };

  // command mode switch variables
  std::vector<bool> stop_modes_;
  std::vector<control_mode_t> start_modes_;

  // active control mode for each actuator
  std::vector<control_mode_t> control_mode_;
};

}  // namespace teknic_hardware

#endif  // TEKNIC_HARDWARE__SYSTEM_HPP_

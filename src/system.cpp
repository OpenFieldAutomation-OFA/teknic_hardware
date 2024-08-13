#include "teknic_hardware/system.hpp"

#include <cmath>
#include <vector>

#include "hardware_interface/types/hardware_interface_type_values.hpp"
#include "rclcpp/rclcpp.hpp"

#define ENABLE_TIMEOUT	3000
#define HOMING_TIMEOUT  50000

namespace teknic_hardware
{
TeknicSystemHardware::~TeknicSystemHardware()
{
  // If the controller manager is shutdown via Ctrl + C
  on_deactivate(rclcpp_lifecycle::State());
  on_cleanup(rclcpp_lifecycle::State());
}

hardware_interface::CallbackReturn TeknicSystemHardware::on_init(
  const hardware_interface::HardwareInfo & info)
{
  if (
    hardware_interface::SystemInterface::on_init(info) !=
    hardware_interface::CallbackReturn::SUCCESS)
  {
    return hardware_interface::CallbackReturn::ERROR;
  }

  hw_states_positions_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_states_velocities_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_states_efforts_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_commands_positions_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  hw_commands_velocities_.resize(info_.joints.size(), std::numeric_limits<double>::quiet_NaN());
  control_mode_.resize(info_.joints.size(), control_mode_t::UNDEFINED);

  for (const hardware_interface::ComponentInfo & joint : info_.joints)
  {
    if (joint.parameters.count("port") != 0 &&
      joint.parameters.count("node") != 0 &&
      joint.parameters.count("vel_limit") != 0 &&
      joint.parameters.count("acc_limit") != 0 &&
      joint.parameters.count("homing") != 0)
    {
      std::pair<std::size_t, std::size_t> node;
      std::string port = joint.parameters.at("port");
      auto it = std::find(chports.begin(), chports.end(), port);
      if (it != chports.end())
      {
        node.first = std::distance(chports.begin(), it);
      }
      else
      {
        node.first = chports.size();
        chports.emplace_back(port);
      }
      node.second = std::stoul(joint.parameters.at("node"));
      nodes.emplace_back(node);

      if (std::stoi(joint.parameters.at("homing")) >= 0 &&
        std::stoi(joint.parameters.at("homing")) <= 2)
      {
        homing_.emplace_back(std::stoi(joint.parameters.at("homing")));
      }
      else
      {
        RCLCPP_FATAL(
          rclcpp::get_logger("CubeMarsSystemHardware"),
          "Homing parameter for joint %s must be 0, 1 or 2", joint.name.c_str());
        return hardware_interface::CallbackReturn::ERROR;
      }
    }
    else
    {
      RCLCPP_FATAL(
        rclcpp::get_logger("CubeMarsSystemHardware"),
        "Missing parameters in URDF for %s", joint.name.c_str());
      return hardware_interface::CallbackReturn::ERROR;
    }

    if (joint.parameters.count("feed_constant") != 0 && std::stod(joint.parameters.at("feed_constant")) > 0)
    {
      counts_conversions_.emplace_back(1 / std::stod(joint.parameters.at("feed_constant")));
      feed_constants_.emplace_back(std::stod(joint.parameters.at("feed_constant")));
    }
    else
    {
      counts_conversions_.emplace_back(1 / (2 * M_PI));
      feed_constants_.emplace_back(0);
    }

    if (joint.parameters.count("peak_torque") != 0 && std::stod(joint.parameters.at("peak_torque")) > 0)
    {
      peak_torques_.emplace_back(std::stod(joint.parameters.at("peak_torque")));
    }
    else
    {
      peak_torques_.emplace_back(0);
    }

    if (joint.parameters.count("read_only") != 0 && std::stoi(joint.parameters.at("read_only")) == 1)
    {
      read_only_.emplace_back(true);
    }
    else
    {
      read_only_.emplace_back(false);
    }
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn TeknicSystemHardware::on_configure(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  try
  {
    for (size_t pc = 0; pc < chports.size(); pc++)
    {
      myMgr->ComHubPort(pc, chports[pc].c_str());
    }
    myMgr->PortsOpen(chports.size());
    for (size_t i = 0; i < chports.size(); i++) {
      sFnd::IPort &myPort = myMgr->Ports(i);
      RCLCPP_INFO(
        rclcpp::get_logger("TeknicSystemHardware"),
        "Port[%d]: state=%d, nodes=%d",
        myPort.NetNumber(), myPort.OpenState(), myPort.NodeCount());
    }
  }
  catch(sFnd::mnErr& theErr)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("TeknicSystemHardware"),
      "Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
		myMgr->PortsClose();
    return hardware_interface::CallbackReturn::FAILURE;
  }

  RCLCPP_INFO(rclcpp::get_logger("TeknicSystemHardware"), "Communication active");

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn TeknicSystemHardware::on_cleanup(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  try
  {
  	myMgr->PortsClose();
  }
  catch(sFnd::mnErr& theErr)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("TeknicSystemHardware"),
      "Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
    return hardware_interface::CallbackReturn::FAILURE;
  }

  RCLCPP_INFO(rclcpp::get_logger("TeknicSystemHardware"), "Communication closed");

  return hardware_interface::CallbackReturn::SUCCESS;
}

std::vector<hardware_interface::StateInterface>
TeknicSystemHardware::export_state_interfaces()
{
  std::vector<hardware_interface::StateInterface> state_interfaces;
  for (std::size_t i = 0; i < info_.joints.size(); i++)
  {
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_POSITION, &hw_states_positions_[i]));
    state_interfaces.emplace_back(hardware_interface::StateInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_states_velocities_[i]));
    if (peak_torques_[i] != 0)
    {
      state_interfaces.emplace_back(hardware_interface::StateInterface(
        info_.joints[i].name, hardware_interface::HW_IF_EFFORT, &hw_states_efforts_[i]));
    }
  }
  return state_interfaces;
}

std::vector<hardware_interface::CommandInterface>
TeknicSystemHardware::export_command_interfaces()
{
  std::vector<hardware_interface::CommandInterface> command_interfaces;
  for (std::size_t i = 0; i < info_.joints.size(); i++)
  {
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, hardware_interface::HW_IF_POSITION, &hw_commands_positions_[i]));
    command_interfaces.emplace_back(hardware_interface::CommandInterface(
      info_.joints[i].name, hardware_interface::HW_IF_VELOCITY, &hw_commands_velocities_[i]));
  }

  return command_interfaces;
}

hardware_interface::return_type TeknicSystemHardware::prepare_command_mode_switch(
  const std::vector<std::string> & start_interfaces,
  const std::vector<std::string> & stop_interfaces)
{
  stop_modes_.clear();
  start_modes_.clear();

  stop_modes_.resize(info_.joints.size(), false);

  // Define allowed combination of command interfaces
  std::unordered_set<std::string> vel {"velocity"};
  std::unordered_set<std::string> pos {"position"};
  
  std::unordered_set<std::string> joint_interfaces;
  for (std::size_t i = 0; i < info_.joints.size(); i++)
  {
    // find stop modes
    for (std::string key : stop_interfaces)
    {
      RCLCPP_INFO(rclcpp::get_logger("CubeMarsSystemHardware"), "stop interface: %s", key.c_str());
      if (key.find(info_.joints[i].name) != std::string::npos)
      {
        stop_modes_[i] = true;
        break;
      }
    }

    // find start modes
    joint_interfaces.clear();
    for (std::string key : start_interfaces)
    {
      RCLCPP_INFO(rclcpp::get_logger("CubeMarsSystemHardware"), "start interface: %s", key.c_str());
      if (key.find(info_.joints[i].name) != std::string::npos)
      {
        joint_interfaces.insert(key.substr(key.find("/") + 1));
      }
    }
    if (joint_interfaces == vel)
    {
      start_modes_.push_back(SPEED_LOOP);
    }
    else if (joint_interfaces == pos)
    {
      start_modes_.push_back(POSITION_LOOP);
    }
    else if (joint_interfaces.empty())
    {
      if (stop_modes_[i])
      {
        start_modes_.push_back(UNDEFINED);
      }
      else
      {
        // don't change control mode
        start_modes_.push_back(control_mode_[i]);
      }
    }
    else
    {
      return hardware_interface::return_type::ERROR;
    }
  }

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type TeknicSystemHardware::perform_command_mode_switch(
  const std::vector<std::string> & /*start_interfaces*/,
  const std::vector<std::string> & /*stop_interfaces*/)
{
  for (std::size_t i = 0; i < info_.joints.size(); i++)
  {
    if (stop_modes_[i])
    {
      hw_commands_velocities_[i] = std::numeric_limits<double>::quiet_NaN();
      hw_commands_positions_[i] = std::numeric_limits<double>::quiet_NaN();
    }
    // switch control mode
    control_mode_[i] = start_modes_[i];
  }
  return hardware_interface::return_type::OK;
}


hardware_interface::CallbackReturn TeknicSystemHardware::on_activate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  try
  {
    for (std::size_t i = 0; i < info_.joints.size(); i++)
    {
      std::pair<std::size_t, std::size_t> node = nodes[i];
      sFnd::INode &inode = myMgr->Ports(node.first).Nodes(node.second);

      // enable node
      RCLCPP_INFO(
        rclcpp::get_logger("TeknicSystemHardware"),
        "Node[%zu]: type=%d\nuserID: %s\nFW version: %s\nSerial #: %d\nModel: %s\n",
        node.first, inode.Info.NodeType(), inode.Info.UserID.Value(),
        inode.Info.FirmwareVersion.Value(), inode.Info.SerialNumber.Value(),
        inode.Info.Model.Value());
      inode.Status.AlertsClear();
      inode.Motion.NodeStopClear();
      inode.EnableReq(true);
      double timeout = myMgr->TimeStampMsec() + ENABLE_TIMEOUT;	//define a timeout in case the node is unable to enable
      while (!inode.Motion.IsReady()) {
        if (myMgr->TimeStampMsec() > timeout) {
          if (inode.Status.Power.Value().fld.InBusLoss) {
            RCLCPP_ERROR(
              rclcpp::get_logger("TeknicSystemHardware"),
              "Bus Power low");
            return hardware_interface::CallbackReturn::ERROR;
          }
          RCLCPP_ERROR(
            rclcpp::get_logger("TeknicSystemHardware"),
            "Timed out waiting for Node %zu to enable", node.first);
          return hardware_interface::CallbackReturn::ERROR;
        }
      }
      RCLCPP_INFO(
        rclcpp::get_logger("TeknicSystemHardware"),
        "Node %zu enabled", node.first);
      
      // homing
      if (homing_[i] != 0)
      {
        if (inode.Motion.Homing.HomingValid())
        {
          if (homing_[i] == 1 && inode.Motion.Homing.WasHomed())
          {
            RCLCPP_INFO(
              rclcpp::get_logger("TeknicSystemHardware"),
              "Node %zu has already been homed, not homing. Current position is: \t%f",
              node.first, inode.Motion.PosnMeasured.Value());
          }
          else
          {
            RCLCPP_INFO(
              rclcpp::get_logger("TeknicSystemHardware"),
              "Homing Node %zu now...", node.first);
            inode.Motion.Homing.Initiate();
            timeout = myMgr->TimeStampMsec() + HOMING_TIMEOUT;	//define a timeout in case the node is unable to enable
            while (!inode.Motion.Homing.WasHomed()) {
              if (myMgr->TimeStampMsec() > timeout) {
                if (inode.Status.Power.Value().fld.InBusLoss) {
                  RCLCPP_ERROR(
                    rclcpp::get_logger("TeknicSystemHardware"),
                    "Bus Power low");
                  return hardware_interface::CallbackReturn::ERROR;
                }
                RCLCPP_ERROR(
                  rclcpp::get_logger("TeknicSystemHardware"),
                  "Node did not complete homing:  \n\t -Ensure Homing settings have been defined through ClearView. \n\t -Check for alerts/Shutdowns \n\t -Ensure timeout is longer than the longest possible homing move");
                return hardware_interface::CallbackReturn::ERROR;
              }
            }
            RCLCPP_INFO(
              rclcpp::get_logger("TeknicSystemHardware"),
              "Node completed homing.");
          }
          
        }
        else {
          RCLCPP_INFO(
            rclcpp::get_logger("TeknicSystemHardware"),
            "Node[%zu] has not had homing setup through ClearView. The node will not be homed.", node.first);
        }
      }

      // enable "interrupting moves"
      inode.Info.Ex.Parameter(98, 1);

      // get encoder counts
      counts_conversions_[i] *= inode.Info.PositioningResolution.Value();      

      // set units
      inode.AccUnit(sFnd::INode::COUNTS_PER_SEC2);
			inode.VelUnit(sFnd::INode::COUNTS_PER_SEC);
			inode.TrqUnit(sFnd::INode::PCT_MAX);   

      // set limits
      double vel = std::stod(info_.joints[i].parameters.at("vel_limit"));
      inode.Motion.VelLimit = vel * counts_conversions_[i];
      double acc = std::stod(info_.joints[i].parameters.at("acc_limit"));
      inode.Motion.AccLimit = acc * counts_conversions_[i];

      double vellim = inode.Motion.VelLimit;
      double accellim = inode.Motion.AccLimit;
      RCLCPP_INFO(
        rclcpp::get_logger("TeknicSystemHardware"),
        "Acceleration limit of Node %zu set to: %f counts/s",
        node.first, accellim);
      RCLCPP_INFO(
        rclcpp::get_logger("TeknicSystemHardware"),
        "Velocity limit of Node %zu set to: %f counts/s^2",
        node.first, vellim);
      
      if (read_only_[i])
      {
        // disable node
        RCLCPP_INFO(
          rclcpp::get_logger("TeknicSystemHardware"),
          "Disabling Node %zu", node.first);
        inode.EnableReq(false);
      }
    }
  }
  catch(sFnd::mnErr& theErr)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("TeknicSystemHardware"),
      "Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
    return hardware_interface::CallbackReturn::ERROR;
  }

  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::CallbackReturn TeknicSystemHardware::on_deactivate(
  const rclcpp_lifecycle::State & /*previous_state*/)
{
  try
  {
    for (std::size_t i = 0; i < info_.joints.size(); i++)
    {
      std::pair<std::size_t, std::size_t> node = nodes[i];
      sFnd::INode &inode = myMgr->Ports(node.first).Nodes(node.second);

      // disable node
      RCLCPP_INFO(
        rclcpp::get_logger("TeknicSystemHardware"),
        "Disabling Node %zu", node.first);
      inode.EnableReq(false);
    }
  }
  catch(sFnd::mnErr& theErr)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("TeknicSystemHardware"),
      "Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
    return hardware_interface::CallbackReturn::ERROR;
  }
  return hardware_interface::CallbackReturn::SUCCESS;
}

hardware_interface::return_type TeknicSystemHardware::read(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  try
  {
    for (std::size_t i = 0; i < info_.joints.size(); i++)
    {
      std::pair<std::size_t, std::size_t> node = nodes[i];
      sFnd::INode &inode = myMgr->Ports(node.first).Nodes(node.second);
      inode.Motion.PosnMeasured.Refresh();
      hw_states_positions_[i] = inode.Motion.PosnMeasured.Value() / counts_conversions_[i];
      inode.Motion.VelMeasured.Refresh();
      hw_states_velocities_[i] = inode.Motion.VelMeasured.Value() / counts_conversions_[i];
      if (peak_torques_[i] != 0)
      {
        inode.Motion.TrqMeasured.Refresh();
        double torque = inode.Motion.TrqMeasured.Value() / 100 * peak_torques_[i];
        if (feed_constants_[i] != 0)
        {
          hw_states_efforts_[i] = torque * 2 * M_PI / feed_constants_[i];
        }
        else
        {
          hw_states_efforts_[i] = torque;
        }
      }

      if (read_only_[i])
      {
        // RCLCPP_INFO(
        //   rclcpp::get_logger("TeknicSystemHardware"),
        //   "pos: %f, vel: %f, torque: %f",
        //   hw_states_positions_[i], hw_states_velocities_[i], hw_states_efforts_[i]);
        RCLCPP_INFO(
          rclcpp::get_logger("TeknicSystemHardware"),
          "Joint %lu: pos: %f",
          i, hw_states_positions_[i]);
      }
    }
  }
  catch(sFnd::mnErr& theErr)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("TeknicSystemHardware"),
      "Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
    return hardware_interface::return_type::ERROR;
  }

  return hardware_interface::return_type::OK;
}

hardware_interface::return_type TeknicSystemHardware::write(
  const rclcpp::Time & /*time*/, const rclcpp::Duration & /*period*/)
{
  try
  {
    for (std::size_t i = 0; i < info_.joints.size(); i++)
    {
      if (!read_only_[i])
      {
        std::pair<std::size_t, std::size_t> node = nodes[i];
        sFnd::INode &inode = myMgr->Ports(node.first).Nodes(node.second);
        switch (control_mode_[i])
        {
          case UNDEFINED:
          {
            // RCLCPP_INFO(
            //   rclcpp::get_logger("TeknicSystemHardware"),
            //   "Nothing is using the hardware interface!");
            break;
          }
          case SPEED_LOOP:
          {
            if (!std::isnan(hw_commands_velocities_[i]))
            {
              double target = hw_commands_velocities_[i] * counts_conversions_[i];
              // RCLCPP_INFO(
              //   rclcpp::get_logger("TeknicSystemHardware"),
              //   "target vel: %i", target);
              inode.Motion.MoveVelStart(target);
            }
            break;
          }
          case POSITION_LOOP:
          {
            if (!std::isnan(hw_commands_positions_[i]))
            {
              double target = hw_commands_positions_[i] * counts_conversions_[i];
              // RCLCPP_INFO(
              //   rclcpp::get_logger("TeknicSystemHardware"),
              //   "target pos: %i", target);
              inode.Motion.MovePosnStart(target, true);
            }
            break;
          }
        }
      }
    }
  }
  catch(sFnd::mnErr& theErr)
  {
    RCLCPP_ERROR(
      rclcpp::get_logger("TeknicSystemHardware"),
      "Caught error: addr=%d, err=0x%08x\nmsg=%s\n", theErr.TheAddr, theErr.ErrorCode, theErr.ErrorMsg);
    return hardware_interface::return_type::ERROR;
  }

  return hardware_interface::return_type::OK;
}

}  // namespace teknic_hardware

#include "pluginlib/class_list_macros.hpp"

PLUGINLIB_EXPORT_CLASS(
  teknic_hardware::TeknicSystemHardware, hardware_interface::SystemInterface)

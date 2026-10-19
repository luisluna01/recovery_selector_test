#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp/bt_factory.h>
#include <behaviortree_cpp/xml_parsing.h>

namespace recovery_selector_test
{

/**
 * @brief Write the behavior tree together with its TreeNodesModel to a single XML in the config
 * directory, so it can be imported into Groot2.
 *
 * The file is named after the XML used to create the behavior tree, with "_models.xml" as the
 * suffix (e.g. "example_traditional_bt.xml" -> "example_traditional_bt_models.xml").
 *
 * @param tree Behavior tree created from the factory.
 * @param bt_xml_name File name of the behavior tree XML (e.g. "example_traditional_bt.xml").
 * @param config_dir Directory the models XML is written to.
 * @param logger Logger used to report the result.
 * @return true if the file was written.
 */
inline bool writeTreeNodesModel(
  const BT::Tree& tree,
  const std::string& bt_xml_name,
  const std::filesystem::path& config_dir,
  const rclcpp::Logger& logger)
{
  const std::filesystem::path models_path =
    config_dir / (std::filesystem::path(bt_xml_name).stem().string() + "_models.xml");

  std::error_code error;
  std::filesystem::create_directories(config_dir, error);

  std::ofstream models_file(models_path);
  if (error || !models_file)
  {
    RCLCPP_WARN(logger, "Could not write TreeNodesModel to '%s'", models_path.c_str());
    return false;
  }

  models_file << BT::WriteTreeToXML(tree, false, false);
  RCLCPP_INFO(logger, "Wrote TreeNodesModel to '%s'", models_path.c_str());
  return true;
}

}  // namespace recovery_selector_test

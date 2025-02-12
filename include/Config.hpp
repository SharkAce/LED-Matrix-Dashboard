#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <filesystem>
#include <fstream>
#include <nlohmann/json_fwd.hpp>
#include <led-matrix.h>

namespace Config {

std::string getProjectRoot();

nlohmann::json getConfig(const std::string& userConfigPath);

rgb_matrix::RGBMatrix::Options getMatrixOptions(const nlohmann::json& globalConfig);

} // namespace Config

#endif // CONFIG_HPP

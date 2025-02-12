#include "pch.hpp"
#include "Config.hpp"

namespace Config {

std::string getProjectRoot() {
	std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
	return exePath.parent_path().parent_path().string().append("/");
}

nlohmann::json getConfig(const std::string& userConfigPath) {
	std::vector<std::string> configPaths;
	if (userConfigPath != "") configPaths.push_back(userConfigPath);
	configPaths.push_back("/etc/matrix-dashboard/config.json");
	configPaths.push_back(getProjectRoot().append("config/config.json"));

	std::ifstream configStream;
	for (auto configPath : configPaths) {
		configStream.open(configPath);
		if (configStream.good()) {
			return nlohmann::json::parse(configStream);
		}
	}

	throw std::runtime_error("Config file not found");
}

rgb_matrix::RGBMatrix::Options getMatrixOptions(const nlohmann::json& globalConfig) {
	nlohmann::json matrixConfig = globalConfig.at("matrix");
	rgb_matrix::RGBMatrix::Options matrixOptions;

	if (matrixConfig.contains("size")) {
		matrixOptions.rows = matrixConfig.at("size").at("rows").get<int>();
		matrixOptions.cols = matrixConfig.at("size").at("cols").get<int>();
	}
	if (matrixConfig.contains("brightness")) {
		matrixOptions.brightness = matrixConfig.at("brightness").get<int>();
	}
	if (matrixConfig.contains("limit_refresh_rate_hz")) {
		matrixOptions.limit_refresh_rate_hz = matrixConfig.at("limit_refresh_rate_hz").get<int>();
	}
	std::string led_rgb_sequence;
	if (matrixConfig.contains("led_rgb_sequence")) {
		led_rgb_sequence = matrixConfig.at("led_rgb_sequence").get<std::string>();
		matrixOptions.led_rgb_sequence = led_rgb_sequence.c_str();
	}

	return matrixOptions;
}

} // namespace Config

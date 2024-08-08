#include "pch.hpp"
#include "Block.hpp"
#include <signal.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <stdexcept>

using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;
using rgb_matrix::Color;
using rgb_matrix::Font;
using nlohmann::json;

std::string getProjectRoot() {
	std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
	return exePath.parent_path().parent_path().string().append("/");
}

json getConfig() {
	const std::string etcPath = "/etc/matrix-dashboard/config.json";
	const std::string localPath = getProjectRoot().append("config/config.json");

	std::ifstream configStream(etcPath);
	if (configStream.good()) {
		return json::parse(configStream);
	}

	configStream.open(localPath);
	if (configStream.good()) {
		return json::parse(configStream);
	}

	throw std::runtime_error("Config file not found");
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

int main(int argc, char *argv[]){
	json config = getConfig();

	json matrixConfig = config.at("matrix");
	RGBMatrix::Options matrixOptions;
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
	if (matrixConfig.contains("led_rgb_sequence")) {
		std::string led_rgb_sequence = matrixConfig.at("led_rgb_sequence").get<std::string>();
		matrixOptions.led_rgb_sequence = led_rgb_sequence.c_str();
	}

	Font font;
	std::string fontPath = getProjectRoot()
			.append("rpi-rgb-led-matrix/fonts/")
			.append((matrixConfig.at("font").get<std::string>()));
	if (!font.LoadFont(fontPath.c_str())) {
		throw std::runtime_error("Could not load font");
	}
	
	RGBMatrix* matrix = RGBMatrix::CreateFromFlags(&argc, &argv, &matrixOptions);
	if (matrix == NULL) {
		return 1;
	}
	FrameCanvas* offscreen = matrix->CreateFrameCanvas();

	auto blocks = Block::createBlocksFromJson(config.at("blocks"), font);

	std::chrono::milliseconds interval(100);
	auto nextTime = std::chrono::steady_clock::now();

	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	while(!interrupt_received){
		offscreen->Clear();
		for (Block& block : blocks) {
			block.update();
			block.draw(offscreen);
		}

		std::this_thread::sleep_until(nextTime);

		offscreen = matrix->SwapOnVSync(offscreen);

		nextTime += interval;
	}
	matrix->Clear();
	delete matrix;
	return 0;
}

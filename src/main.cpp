#include "pch.hpp"
#include "Block.hpp"
#include <signal.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>

using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;
using rgb_matrix::Color;
using rgb_matrix::Font;
using nlohmann::json;

std::string getProjectRoot() {
	std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
	return exePath.parent_path().parent_path().string().append("/");
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

int main(int argc, char *argv[]){
	std::string configPath = getProjectRoot().append("config/config.json");
	std::ifstream jsonFile(configPath);
	json config = json::parse(jsonFile);

	json matrixConfig = config.at("matrixConfig");
	RGBMatrix::Options matrixOptions;
	matrixOptions.rows = matrixConfig.at("rows").get<int>();
	matrixOptions.cols = matrixConfig.at("cols").get<int>();
	matrixOptions.brightness = matrixConfig.at("brightness").get<int>();
	matrixOptions.show_refresh_rate = matrixConfig.at("show_refresh_rate").get<bool>();
	matrixOptions.limit_refresh_rate_hz = matrixConfig.at("limit_refresh_rate_hz").get<int>();
	std::string led_rgb_sequence = matrixConfig.at("led_rgb_sequence").get<std::string>();
	matrixOptions.led_rgb_sequence = led_rgb_sequence.c_str();

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

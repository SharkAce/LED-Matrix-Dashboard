#include "pch.hpp"
#include "Block.hpp"
#include <signal.h>
#include <filesystem>
#include <chrono>
#include <thread>

using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;
using rgb_matrix::Color;
using rgb_matrix::Font;

std::string getProjectRoot() {
	std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
	return exePath.parent_path().parent_path().string().append("/");
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

int main(int argc, char *argv[]){
	Font font;
	std::string fontPath = getProjectRoot().append("rpi-rgb-led-matrix/fonts/4x6.bdf");
	font.LoadFont(fontPath.c_str());

	std::string configPath = getProjectRoot().append("config/config.json");

	// My defaults change it accordingly
	RGBMatrix::Options matrixOptions;
	matrixOptions.hardware_mapping = "regular";
	matrixOptions.led_rgb_sequence = "RBG";
	matrixOptions.rows = 32;
	matrixOptions.cols = 64;
	matrixOptions.chain_length = 1;
	matrixOptions.parallel = 1;
	matrixOptions.show_refresh_rate = false;
	matrixOptions.limit_refresh_rate_hz = 120;
	matrixOptions.brightness = 50;
	matrixOptions.disable_hardware_pulsing = false;

	RGBMatrix* matrix = RGBMatrix::CreateFromFlags(&argc, &argv, &matrixOptions);
	if (matrix == NULL) {
		return 1;
	}
	FrameCanvas* offscreen = matrix->CreateFrameCanvas();

	auto blocks = Block::createBlocksFromJson(configPath, font);

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

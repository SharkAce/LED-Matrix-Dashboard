#include <led-matrix.h>
#include <graphics.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <filesystem>
#include <Block.hpp>

using rgb_matrix::RGBMatrix;
using rgb_matrix::Color;
using rgb_matrix::Font;
const int microsecond = 1000000;

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
	matrixOptions.brightness = 50;
	matrixOptions.disable_hardware_pulsing = false;

	RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &matrixOptions);

	auto blocks = Block::createBlocksFromJson(configPath, font);

	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	while(!interrupt_received){
		canvas->Clear();

		for (Block& block : blocks) {
			block.update();
			block.draw(canvas);
		}

		usleep(5 * microsecond);
	}
	canvas->Clear();
	delete canvas;
	return 0;
}

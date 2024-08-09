#include "pch.hpp"
#include "Block.hpp"
#include <signal.h>
#include <getopt.h>
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

static int usage(char *progname) {
  fprintf(stderr, "usage: %s [options]\n", progname);
  fprintf(stderr, "Options:\n");
	fprintf(stderr,
			"\t-c <config-file> : Use given config.\n"
			"\t-n               : Dry run. Collect inputs and update\n"
			"\t                   blocks without starting the matrix.\n"
			"\n"
			);
	rgb_matrix::PrintMatrixFlags(stderr);

  return 1;
}

static std::string getProjectRoot() {
	std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
	return exePath.parent_path().parent_path().string().append("/");
}

static json getConfig(const std::string& userConfigPath) {
	std::vector<std::string> configPaths;
	if (userConfigPath != "") configPaths.push_back(userConfigPath);
	configPaths.push_back("/etc/matrix-dashboard/config.json");
	configPaths.push_back(getProjectRoot().append("config/config.json"));

	std::ifstream configStream;
	for (auto configPath : configPaths) {
		configStream.open(configPath);
		if (configStream.good()) {
			return json::parse(configStream);
		}
	}

	throw std::runtime_error("Config file not found");
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

int main(int argc, char *argv[]){
	bool dryrun = false;
	std::string userConfigPath = "";
	
	int opt;
	while ((opt = getopt(argc, argv, "c:n")) != -1) {
		switch (opt) {
		case 'c': userConfigPath = optarg; break;
		case 'n': dryrun = true; break;
		default:
			return usage(argv[0]);
		}
	}

	json config = getConfig(userConfigPath);
	json matrixConfig = config.at("matrix");

	RGBMatrix::Options matrixOptions;
	rgb_matrix::RuntimeOptions runtimeOptions;

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

	if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
				&matrixOptions, &runtimeOptions)) {
		return usage(argv[0]);
	}

	Font font;
	std::string fontPath = getProjectRoot()
			.append("rpi-rgb-led-matrix/fonts/")
			.append((matrixConfig.at("font").get<std::string>()));
	if (!font.LoadFont(fontPath.c_str())) {
		throw std::runtime_error("Could not load font");
	}

	auto blocks = Block::createBlocksFromJson(config.at("blocks"), font);

	if (dryrun) {
		for (Block& block : blocks) block.update();
		return 0;
	}

	RGBMatrix* matrix = RGBMatrix::CreateFromOptions(matrixOptions, runtimeOptions);
	if (matrix == NULL) {
		return 1;
	}
	FrameCanvas* offscreen = matrix->CreateFrameCanvas();

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

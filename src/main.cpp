#include "pch.hpp"
#include "Block.hpp"
#include "Config.hpp"
#include <signal.h>
#include <getopt.h>
#include <chrono>
#include <thread>
#include <stdexcept>

using rgb_matrix::RGBMatrix;
using rgb_matrix::FrameCanvas;
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

	rgb_matrix::RuntimeOptions runtimeOptions;
	if (dryrun) runtimeOptions.do_gpio_init = false;

	json globalConfig = Config::getConfig(userConfigPath);
	RGBMatrix::Options matrixOptions = Config::getMatrixOptions(globalConfig);

	if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
				&matrixOptions, &runtimeOptions)) {
		return usage(argv[0]);
	}

	Font font;
	std::string fontPath = Config::getProjectRoot()
			.append("fonts/")
			.append((globalConfig.at("matrix").at("font").get<std::string>()));
	if (!font.LoadFont(fontPath.c_str())) {
		throw std::runtime_error("Could not load font");
	}

	auto blocks = Block::createBlocksFromJson(globalConfig.at("blocks"), font);

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

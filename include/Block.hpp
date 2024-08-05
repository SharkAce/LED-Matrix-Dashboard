#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <graphics.h>
#include <content-streamer.h>
#include <chrono>
#include <vector>
#include "TextProvider.hpp"

class Block {
public:
	struct Config {
		std::unique_ptr<TextProvider::Base> textProvider;
		rgb_matrix::Font& font;
		rgb_matrix::Color color;
		int interval;
		int x;
		int y;
	};

	Block(Config&& config);
	static std::vector<Block> createBlocksFromJson(const std::string& filename, rgb_matrix::Font& font);
	void draw(rgb_matrix::FrameCanvas* matrix);
	void update();

private:
	std::unique_ptr<TextProvider::Base> textProvider;
	rgb_matrix::Font& font;
	rgb_matrix::Color color;
	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
	int interval;
	int x;
	int y;
};

#endif // BLOCK_HPP

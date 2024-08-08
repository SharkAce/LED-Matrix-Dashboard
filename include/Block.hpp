#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <graphics.h>
#include <content-streamer.h>
#include <chrono>
#include <vector>
#include "TextProvider.hpp"

class Block {
public:
	enum Origin {
		TopLeft,
		TopRight,
		BottomLeft,
		BottomRight
	};

	struct Config {
		std::unique_ptr<TextProvider::Base> textProvider;
		rgb_matrix::Font& font;
		rgb_matrix::Color color;
		int interval;
		Origin origin;
		int relativeX;
		int relativeY;
	};

	Block(Config&& config);
	static std::vector<Block> createBlocksFromJson(const nlohmann::json& config, rgb_matrix::Font& font);
	void draw(rgb_matrix::FrameCanvas* matrix);
	void update();

private:
	void setAbsolutePosition(const std::string& text);

	std::unique_ptr<TextProvider::Base> textProvider;
	rgb_matrix::Font& font;
	rgb_matrix::Color color;
	std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime;
	int interval;
	Origin origin;
	int relativeX;
	int relativeY;
	int absoluteX;
	int absoluteY;
};

#endif // BLOCK_HPP

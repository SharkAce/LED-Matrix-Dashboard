#include "pch.hpp"
#include "Block.hpp"

Block::Block(Block::Config&& config) 
	: font(config.font),
	textProvider(std::move(config.textProvider)),
	color(config.color),
	interval(config.interval),
	origin(config.origin),
	relativeX(config.relativeX),
	relativeY(config.relativeY) {};

std::vector<Block> Block::createBlocksFromJson(const nlohmann::json& j, rgb_matrix::Font& font) {
	std::vector<Block> blocks;

	for (const auto& item : j) {
		auto textProvider = TextProvider::createFromJSON(item);

		int r = item.at("color")[0].get<int>();
		int g = item.at("color")[1].get<int>();
		int b = item.at("color")[2].get<int>();
		rgb_matrix::Color color(r, g, b);

		int relativeX = item.at("position").at("x").get<int>();
		int relativeY = item.at("position").at("y").get<int>();

		int interval = item.at("interval").get<int>();

		Block::Origin origin = Block::Origin::TopLeft;
		if (item.contains("origin")) {
			std::string itemOrigin = item.at("origin").get<std::string>();
			if (itemOrigin == "top-right") {
				origin = Block::Origin::TopRight;
			} else if (itemOrigin == "bottom-left") {
				origin = Block::Origin::BottomLeft;
			} else if (itemOrigin == "bottom-right") {
				origin = Block::Origin::BottomRight;
			}
		}

		Block::Config config = {
			.textProvider = std::move(textProvider),
			.font = font,
			.color = color,
			.interval = interval,
			.origin = origin,
			.relativeX = relativeX,
			.relativeY = relativeY
		};

		blocks.emplace_back(std::move(config));
	}

	return blocks;
}

void Block::setAbsolutePosition(const std::string& text) {
	int textHeight = font.height() - 1; 
	int textWidth = -1;

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	std::wstring wideText = converter.from_bytes(text);

	for (wchar_t c : wideText) {
		textWidth += font.CharacterWidth(c);
	}

	int newAbsoluteX = relativeX;
	int newAbsoluteY = relativeY + font.baseline();

	switch (origin) {
		case Origin::TopLeft:
			break;
		case Origin::TopRight:
			newAbsoluteX -= textWidth;
			break;
		case Origin::BottomLeft:
			newAbsoluteY -= textHeight;
			break;
		case Origin::BottomRight:
			newAbsoluteY -= textHeight;
			newAbsoluteX -= textWidth;
			break;
	}

	absoluteX = newAbsoluteX;
	absoluteY = newAbsoluteY;
}

void Block::draw(rgb_matrix::FrameCanvas* canvas) {
	if (textProvider) {
		std::string text = textProvider->getText();
		rgb_matrix::DrawText(canvas, font, absoluteX, absoluteY, color, text.c_str());
	}
}

void Block::update() {
	auto now = std::chrono::steady_clock::now();
	if (textProvider && now - lastUpdateTime >= (std::chrono::seconds)interval) {
		textProvider->update();
		setAbsolutePosition(textProvider->getText());
		lastUpdateTime = now;
	}
}

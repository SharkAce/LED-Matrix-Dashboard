#ifndef TEXT_PROVIDER_HPP
#define TEXT_PROVIDER_HPP

#include <string>
#include <memory>
#include <json.hpp>
#include <stdexcept>
#include <iostream>
#include <curl/curl.h>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace TextProvider {

class Base {
public:
	virtual void update() = 0;
	std::string getText() const {
		return text;
	}
protected:
	std::string text;
};

std::unique_ptr<Base> createFromJSON(const nlohmann::json& j);

class Http : public Base {
public:
	Http(const std::string& url) : url(url) {};
	void update() override;

private:
	const std::string url;
};

class Time : public Base {
public:
	Time(const std::string& format) : format(format) {};
	void update() override;

private:
	const std::string format;
};

} // namespace TextProvider

#endif

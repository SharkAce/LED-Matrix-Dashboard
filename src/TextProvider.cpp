#include <TextProvider.hpp>

namespace TextProvider {

	std::unique_ptr<TextProvider::Base> createFromJSON(const nlohmann::json& j) {
		std::string type = j.at("type").get<std::string>();

		if (type == "http") {
			const std::string url = j.at("url").get<std::string>();
			return std::make_unique<Http>(url);
		} else if (type == "time") {
			const std::string format = j.at("format").get<std::string>();
			return std::make_unique<Time>(format);
		} else {
			throw std::runtime_error("Unknown TextProvider type");
		}
	};

	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	void Http::update() {
		CURL* curl;
		CURLcode res;
		std::string readBuffer;

		curl = curl_easy_init();
		if(curl) {
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str()); // Set the URL
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Set callback function
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // Pass the string to write data to
			res = curl_easy_perform(curl); // Perform the request

			if(res != CURLE_OK) {
				std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
				curl_easy_cleanup(curl);
				return;
			}
			curl_easy_cleanup(curl);
		}

		text = readBuffer;
	}

	void Time::update() {
		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);

		std::ostringstream oss;
		oss << std::put_time(localTime, format.c_str());
		text = oss.str();
	}

} // namespace TextProvider

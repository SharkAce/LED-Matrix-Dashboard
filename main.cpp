#include "rpi-rgb-led-matrix/include/led-matrix.h"
#include "rpi-rgb-led-matrix/include/graphics.h"
#include <curl/curl.h>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <ctime>
#include <filesystem>

std::string getExecutablePath() {
	std::filesystem::path exePath = std::filesystem::canonical("/proc/self/exe");
	return exePath.parent_path().string();
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

std::string fetchUrlData(const char* url) {
	CURL* curl;
	CURLcode res;
	std::string readBuffer;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url); // Set the URL
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback); // Set callback function
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer); // Pass the string to write data to
		res = curl_easy_perform(curl); // Perform the request

		if(res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
			curl_easy_cleanup(curl);
			return "";
		}
		curl_easy_cleanup(curl);
	}

	return readBuffer;
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;
using rgb_matrix::Color;
using rgb_matrix::Font;

constexpr const int microsecond = 1000000;
constexpr const char* wttrTempUrl = "https://wttr.in/montreal?format=\%t";
constexpr const char* wttrFeelUrl = "https://wttr.in/montreal?format=\%f";
constexpr const char* publicIpUrl = "https://ipinfo.io/ip";

int main(int argc, char *argv[]){
	RGBMatrix::Options defaults;

	// My defaults change it accordingly
	defaults.hardware_mapping = "regular";
	defaults.led_rgb_sequence = "RBG";
	defaults.rows = 32;
	defaults.cols = 64;
	defaults.chain_length = 1;
	defaults.parallel = 1;
	defaults.show_refresh_rate = false;
	defaults.brightness = 50;
	defaults.disable_hardware_pulsing = false;

	Font font;
	std::string fontPath = getExecutablePath() + "/rpi-rgb-led-matrix/fonts/4x6.bdf";
	font.LoadFont(fontPath.c_str());

	Color color1(64,0,128);
	Color color2(0,128,128);
	Color color3(100,0,100);
	Color color4(0,128,64);

	RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

	char dateString[11]; // Enough space for "YYYY/MM/DD\0"
	char timeString[9];  // Enough space for "HH:MM:SS\0"
	char weekdayString[10]; // Enough space for all weekday names

	std::string addrString;
	std::string tempString;
	std::string feelString;
	std::string pubIpString;

	int timeStep = 0;
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	while(!interrupt_received){
		// 120 * 5 sec = 10 min = 144/day < 250 max calls per day
		if (timeStep % 120 == 0){ 
			tempString = fetchUrlData(wttrTempUrl);
			feelString = fetchUrlData(wttrFeelUrl);
			pubIpString = fetchUrlData(publicIpUrl);
		}

		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);

		std::strftime(dateString, sizeof(dateString), "%Y/%m/%d", localTime);
		std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
		std::strftime(weekdayString, sizeof(weekdayString), "%A", localTime);

		// Round last digit to 5
		int lastTimeDigit = timeString[7] - '0';
		timeString[7] = lastTimeDigit < 5 ? '0' : '5';

		canvas->Clear();
		rgb_matrix::DrawText(canvas, font,2, 2 + font.baseline(),color1, NULL, weekdayString, 0);
		rgb_matrix::DrawText(canvas, font,2, 9 + font.baseline(),color1, NULL, dateString, 0);
		rgb_matrix::DrawText(canvas, font,2, 16 + font.baseline(),color1, NULL, timeString, 0);
		rgb_matrix::DrawText(canvas, font,2, 23 + font.baseline(),color3, NULL, pubIpString.c_str(), 0);
		rgb_matrix::DrawText(canvas, font,65-((tempString.size()-1)*4), 0 + font.baseline(),color2, NULL, tempString.c_str(), 0);
		rgb_matrix::DrawText(canvas, font,65-((feelString.size()-1)*4), 6 + font.baseline(),color4, NULL, feelString.c_str(), 0);
		usleep(5 * microsecond);
		timeStep ++;
	}
	canvas->Clear();
	delete canvas;
	return 0;
}

#include "include/led-matrix.h"
#include "include/graphics.h"
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <vector>
#include <string>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <curl/curl.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
	size_t total_size = size * nmemb;
	output->append(static_cast<char*>(contents), total_size);
	return total_size;
}

std::string getWeatherInfo() {
	// Initialize cURL
	CURL* curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();

	std::string url = "https://wttr.in/montreal?format=\%t";

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

	std::string response;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return response;
}

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

int main(int argc, char *argv[]){
	RGBMatrix::Options defaults;

	// My defaults change yours accordingly
	defaults.hardware_mapping = "regular";
	defaults.led_rgb_sequence = "RBG";
	defaults.rows = 32;
	defaults.cols = 64;
	defaults.chain_length = 1;
	defaults.parallel = 1;
	defaults.show_refresh_rate = false;
	defaults.brightness = 80;
	defaults.disable_hardware_pulsing = true;

	rgb_matrix::Font font1;
	font1.LoadFont("./fonts/4x6.bdf");

	rgb_matrix::Color color1(64,0,128);
	rgb_matrix::Color color2(0,128,128);

	RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

	char timeString[10];
	char dateString[20];
	char weekdayString[20];
	std::string temperatureString;
	int timeStep = 0;

	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	while(!interrupt_received){
		// 120 * 5 sec = 10 min = 144/day < 250 max calls per day
		if (timeStep % 120 == 0){ 
			//temperatureString = getWeatherInfo();
		}
	
		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);

		std::strftime(dateString, sizeof(dateString), "%Y/%m/%d", localTime);
		std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
		std::strftime(weekdayString, sizeof(weekdayString), "%A", localTime);

		// Round last digit to 5
		int lastTimeDigit = timeString[strlen(timeString)-1] - '0';
		timeString[7] = lastTimeDigit < 5 ? '0' : '5';

		canvas->Clear();
		rgb_matrix::DrawText(canvas, font1,0, 0 + font1.baseline(),color1, NULL, weekdayString,0);
		rgb_matrix::DrawText(canvas, font1,0, 7 + font1.baseline(),color1, NULL, dateString,0);
		rgb_matrix::DrawText(canvas, font1,0, 14 + font1.baseline(),color1, NULL, timeString,0);
		rgb_matrix::DrawText(canvas, font1,40, 0 + font1.baseline(),color2, NULL, temperatureString.c_str(),0);
		int microsecond = 1000000;
		usleep(5 * microsecond);
		timeStep ++;
	}
	canvas->Clear();
	delete canvas;
	return 0;
}

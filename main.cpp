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

    if (curl) {
        std::string url = "https://wttr.in/montreal?format=\%t";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        return response;
    } else {
        std::cerr << "Error initializing cURL." << std::endl;
        return "";
    }
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

	// Loading font
	rgb_matrix::Font font1;
	font1.LoadFont("./fonts/4x6.bdf");

	rgb_matrix::Font font2;
	font2.LoadFont("./fonts/5x8.bdf");

	rgb_matrix::Color color1(64,0,128);
	rgb_matrix::Color color2(0,128,128);

	RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);
	char dateString[50];
	char timeString[10];
	char weekdayString[20];
	std::string tempString;
	int timeStep = 0;
	while(!interrupt_received){
		if (timeStep % 600 == 0){ 
			tempString = getWeatherInfo();
		}
	
		std::time_t currentTime = std::time(nullptr);
		std::tm* localTime = std::localtime(&currentTime);

		std::strftime(dateString, sizeof(dateString), "%Y/%m/%d", localTime);
		std::strftime(timeString, sizeof(timeString), "%H:%M:%S", localTime);
		std::strftime(weekdayString, sizeof(weekdayString), "%A", localTime);

		char lastTimeDigit_char = static_cast<char>(timeString[strlen(timeString)-1]);
		int lastTimeDigit_int = lastTimeDigit_char - '0';
		if (lastTimeDigit_int < 5) {
			timeString[7] = '0';
		} else {
			timeString[7] = '5';
		}

		canvas->Clear();
		rgb_matrix::DrawText(canvas, font1,0, 0 + font1.baseline(),color1, NULL, weekdayString,0);
		rgb_matrix::DrawText(canvas, font1,0, 7 + font1.baseline(),color1, NULL, dateString,0);
		rgb_matrix::DrawText(canvas, font1,0, 14 + font1.baseline(),color1, NULL, timeString,0);
		rgb_matrix::DrawText(canvas, font1,40, 0 + font1.baseline(),color2, NULL, tempString.c_str(),0);
		usleep(5000000);
		timeStep ++;
	}
	canvas->Clear();
	delete canvas;
	return 0;
}


#include "matrix/include/led-matrix.h"
#include "matrix/include/graphics.h"
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
#include <cstring>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <curl/curl.h>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

char* fetchUrlData(const char* url) {
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
            return nullptr;
        }
        curl_easy_cleanup(curl);
    }

    // Allocate memory for the result and copy the string content
    char* result = new char[readBuffer.size() + 1];
    strcpy(result, readBuffer.c_str());

    return result;
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

	Font font1;
	font1.LoadFont("./fonts/4x6.bdf");

	Color color1(64,0,128);
	Color color2(0,128,128);
	Color color3(100,0,100);
	Color color4(0,128,64);

	RGBMatrix *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);

	char* timeString;
	char* dateString;
	char* weekdayString;
	char* addrString;
	char* tempString;
	char* feelString;
	char* pubIpString;

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
		int lastTimeDigit = timeString[strlen(timeString)-1] - '0';
		timeString[7] = lastTimeDigit < 5 ? '0' : '5';

		canvas->Clear();
		rgb_matrix::DrawText(canvas, font1,0, 0 + font1.baseline(),color1, NULL, weekdayString,0);
		rgb_matrix::DrawText(canvas, font1,0, 7 + font1.baseline(),color1, NULL, dateString,0);
		rgb_matrix::DrawText(canvas, font1,0, 14 + font1.baseline(),color1, NULL, timeString,0);
		rgb_matrix::DrawText(canvas, font1,2, 23 + font1.baseline(),color3, NULL, pubIpString,0);
		rgb_matrix::DrawText(canvas, font1,40, 0 + font1.baseline(),color2, NULL, tempString,0);
		rgb_matrix::DrawText(canvas, font1,65-((strlen(tempString)-1)*4), 0 + font1.baseline(),color2, NULL, tempString, 0);
		rgb_matrix::DrawText(canvas, font1,65-((strlen(feelString)-1)*4), 6 + font1.baseline(),color4, NULL, feelString, 0);
		usleep(5 * microsecond);
		timeStep ++;
	}
	canvas->Clear();
	delete canvas;
	return 0;
}

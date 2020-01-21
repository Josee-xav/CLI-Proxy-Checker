#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <tuple>  
#include <regex>

#define CURL_STATICLIB
#include <curl.h>
#include <mutex>

class ProxyTester
{
public:
	ProxyTester(std::string protocol);
	void checkProxyConnection(std::string ip, int port);

	std::string getTestedIpDetails(int pos);

	int getWorkingProxyStat();


	int getDeadProxyStat();


	int getTimedOutProxyStat();

private:
	/*
	* This function gets called by libcurl as soon as there is data received that needs to be saved.
	* The size of the data pointed to by char* data is size multiplied with size_t nmemb, it will not be zero terminated.
	* Return the number of bytes actually taken care of.
	* If that amount differs from the amount passed to your function, it'll signal an error to the library.
	* This will exit the transferand return CURLE_WRITE_ERROR.
	*/
	static int writer(char* data, size_t size, size_t nmemb, std::string* buffer);

	// writes the working proxy to a file called WorkingProxies.txt
	inline void writeWorkingProxies(std::string workingProxy)
	{
		std::ofstream workingProxyFile("WorkingProxies.txt", std::ios::app);

		workingProxyFile << workingProxy << "\n";
	}
	int workingProxies;
	int timedOutProxies;
	int deadProxies;

	std::vector<std::string> testedIpsWithDetails;
	std::string proxyProtocol;
};


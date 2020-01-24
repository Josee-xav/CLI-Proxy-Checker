#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <cstdlib> 
#include <mutex>

#define CURL_STATICLIB
#include <curl.h>

#define WORKING_PROXIES_FILE "WorkingProxies.txt"

class ProxyTester
{
public:
	ProxyTester(std::string protocol);
	void checkProxyConnection(std::string ip, int port);

	inline int getWorkingProxyStat()
	{
		return workingProxies;
	}

	inline int getDeadProxyStat()
	{
		return deadProxies;
	}

	inline int getTimedOutProxyStat()
	{
		return timedOutProxies;
	}

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
		std::ofstream workingProxyFile(WORKING_PROXIES_FILE, std::ios::app);

		if(!workingProxyFile) {
			std::cerr << "A file could not be opened!\n";
			exit(1);
		}
		workingProxyFile << workingProxy << "\n";
	}

	int workingProxies;
	int timedOutProxies;
	int deadProxies;

	std::string proxyProtocol;
};


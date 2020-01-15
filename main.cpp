#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <tuple>  
#include <regex>


#define CURL_STATICLIB
#include <curl.h>

namespace testedProxiesStats
{
	int working = 0;
	int timedOut = 0;
	int dead = 0;
	std::vector<std::string> testedIpsWithDetails;
}

/*
* This function gets called by libcurl as soon as there is data received that needs to be saved.
* The size of the data pointed to by char* data is size multiplied with size_t nmemb, it will not be zero terminated.
* Return the number of bytes actually taken care of.
* If that amount differs from the amount passed to your function, it'll signal an error to the library.
* This will exit the transferand return CURLE_WRITE_ERROR.
*/
static int writer(char* data, size_t size, size_t nmemb, std::string* buffer)
{
	unsigned int result = 0;
	if(buffer != NULL) {
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

inline void writeWorkingProxies(std::string workingProxy)
{
	std::ofstream workingProxyFile("WorkingProxies.txt", std::ios::app);

	workingProxyFile << workingProxy << "\n";
}

/*checkProxyConnection
	This function checks the connection of the proxy.

	If the connection times out on the first loop then on the 2nd loop it will try again if it still times out then it will return ip + " : " + std::to_string(port) + " Failed TimedOut After 2 attempts of connecting...."
*/
void checkProxyConnection(std::string ip, int port, std::string proxytype)
{
	std::string buffer; // string used by writer function to save cURL
	CURL* curlHandle;
	CURLcode curlResponse;
	curlHandle = curl_easy_init();

	if(curlHandle) {
		curl_easy_setopt(curlHandle, CURLOPT_URL, "https://www.google.com");
		curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);
		curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 12L);
		curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:69.0) Gecko/20100101 Firefox/69.0");

		// sets the proxyType
		if(proxytype == "http")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
		else if(proxytype == "https")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
		else if(proxytype == "socks4")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
		else if(proxytype == "socks5")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

		curl_easy_setopt(curlHandle, CURLOPT_PROXY, ip.c_str());
		curl_easy_setopt(curlHandle, CURLOPT_PROXYPORT, port);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &buffer);

		curlResponse = curl_easy_perform(curlHandle); // will perform the transfer as described in all of the options above
		curl_easy_cleanup(curlHandle);

		if(curlResponse != CURLE_OPERATION_TIMEDOUT) {
			if(curlResponse == CURLE_OK) {
				if(sizeof(buffer) > 0) {// receviced data thats greater than 0. meaning that the connection went though
					writeWorkingProxies(ip + " : " + std::to_string(port));

					testedProxiesStats::working++;
					testedProxiesStats::testedIpsWithDetails.push_back(ip + " : " + std::to_string(port) + " worked!");
				}
			}
			else {
				testedProxiesStats::dead++;
				testedProxiesStats::testedIpsWithDetails.push_back(ip + " : " + std::to_string(port) + " Failed Reason : " + static_cast<std::string>(curl_easy_strerror(curlResponse)));
			}
		}
		else {
			testedProxiesStats::timedOut++;
			testedProxiesStats::testedIpsWithDetails.push_back(ip + " : " + std::to_string(port) + " Failed Reason: Connection Timed out");
		}
	}
	else {
		std::cout << "The Curl handle Failed! Press a letter to exit\n"; // error
		std::cin.get();
		exit(1);
	}
}

/* readTextFile
	This function uses a textfile name provided by the user thats stored in p_fileName.
	The fileName will be used so it can read the IP's and the port of that ip and then puts it puts that data into one of the vectors so the IPS will go to
	string vector "p_ips" and the ports will go to a int vector called "p_port"

	parameter:
	p_fileName is the filename that you want to read from

	Return:
	Returns a tuple of std::string vector and a vector of ints :)
*/
std::tuple<std::vector<std::string>, std::vector<int>> readTextFile(std::string p_fileName)
{
	std::regex regexPort("^[0-9]+$");
	std::regex regexIp("^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$");

	std::ifstream rFile(p_fileName);

	// If we couldn't open the output file stream for reading
	if(!rFile) {
		std::cerr << "Uh oh, File: " << p_fileName << " could not be opened for reading! Press a letter to exit" << std::endl;
		std::cin.get();
		exit(1);
	}
	std::vector<std::string>ips;
	std::vector<int>ports;

	while(rFile) {
		std::string stringPort;
		std::string ip;
		std::getline(rFile, ip, ':');
		std::getline(rFile, stringPort);

		if(std::regex_match(ip, regexIp)) {
			std::cout << "worked : " << ip << "\n";
			ips.push_back(ip);
		}

		if(std::regex_match(stringPort, regexPort))
			ports.push_back(std::stoi(stringPort));
	}
	return std::make_tuple(ips, ports);
}

void drawProxyWorkingGrid()
{
	system("cls");

	std::cout << "-- Working\n";
	std::cout << testedProxiesStats::working;
	std::cout << "\n-- TimedOut\n";
	std::cout << testedProxiesStats::timedOut;
	std::cout << "\n-- Not Working\n";
	std::cout << testedProxiesStats::dead;

	std::cout << "\n-- IPS TESTED\n";
	for(int i = 0; i < testedProxiesStats::testedIpsWithDetails.size(); i++)
		std::cout << testedProxiesStats::testedIpsWithDetails.at(i) << ", " << i << "\n";
}

int main()
{
	std::vector<std::string> ipsVector;
	std::vector<int> portVector;

	std::string fileName{};
	std::string proxiesProtocol{};

	std::cout << "Whats the file name that contains all of the ips. example: proxyList.txt\n";
	std::getline(std::cin, fileName);

	do {
		std::cout << "Whats the type the protocol that all of the proxies are. The protocols are: http, https, socks4, socks5\n";
		std::getline(std::cin, proxiesProtocol);
	} while(proxiesProtocol != "http" && proxiesProtocol != "https" && proxiesProtocol != "socks4" && proxiesProtocol != "socks5");

	std::tie(ipsVector, portVector) = readTextFile(fileName);

	for(int i = 0; i < ipsVector.size(); i++) {
		checkProxyConnection(ipsVector.at(i), portVector.at(i), proxiesProtocol);
		drawProxyWorkingGrid();
	}

	std::cout << "Finished..... Press a letter to exit : ";
	std::cin.get();
}

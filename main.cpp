#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <tuple>  
#include <regex>
#include <thread>

#include "ProxyTester.h"
#include "Timer.h"

#define CURL_STATICLIB
#include <curl.h>

constexpr int THREAD_COUT{ 3 };

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
	std::regex regexIp("^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$");

	std::ifstream rFile(p_fileName);

	// If we couldn't open the output file stream for reading
	if(!rFile) {
		std::cerr << p_fileName << " could not be opened! Press a letter to exit\n";
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

		if(std::regex_match(ip, regexIp) && std::regex_match(stringPort, regexPort)) {
			ips.push_back(ip);
			ports.push_back(std::stoi(stringPort));
		}
	}

	if(ips.size() == 0 && ports.size() == 0) {
		std::cerr << "Couldnt find any ips and ports.. Press a letter to exit\n";
		std::cin.get();
		exit(1);
	}
	else
		return std::make_tuple(ips, ports);
}

void drawProxyDetails(int amountWorking, int amountTimedOut, int amountDead)
{
	std::cout << "STATS---- Working " << amountWorking << " -- TimedOut " << amountTimedOut << " -- Not Working " << amountDead << "\n";

}

int main()
{
	std::vector<std::string> ipsVector;
	std::vector<int> portVector;

	std::string fileName;
	std::string proxiesProtocol;

	std::cout << "Whats the file name that contains all of the ips. example: proxyList.txt\n";
	std::getline(std::cin, fileName);

	do {
		std::cout << "Whats the type the protocol that all of the proxies are. The protocols are: http, https, socks4, socks5\n";
		std::getline(std::cin, proxiesProtocol);
	} while(proxiesProtocol != "http" && proxiesProtocol != "https" && proxiesProtocol != "socks4" && proxiesProtocol != "socks5");

	std::tie(ipsVector, portVector) = readTextFile(fileName);

	ProxyTester test(proxiesProtocol);
	Timer r;

	std::thread tid[THREAD_COUT];
	curl_global_init(CURL_GLOBAL_ALL);
	int ipI = 0;
	do {
		// ipI < ipsVector.size() prevents out of range error.
		for(int i = 0; i < THREAD_COUT && ipI < ipsVector.size(); i++, ++ipI)
			tid[i] = std::thread(&ProxyTester::checkProxyConnection, &test, ipsVector[ipI], portVector[ipI]);

		/* now wait for all threads to terminate*/
		for(int i = 0; i < THREAD_COUT; i++) {
			if(tid[i].joinable()) {
				tid[i].join();
			}
		}

		drawProxyDetails(test.getWorkingProxyStat(), test.getTimedOutProxyStat(), test.getDeadProxyStat());
	} while(ipI < ipsVector.size());

	std::cout << "time elpased: " << r.elapsed() << "\n";
	std::cout << "Finished..... Press a letter to exit : ";
	std::cin.get();
}

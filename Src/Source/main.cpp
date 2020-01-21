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

inline void writeWorkingProxies(std::string workingProxy)
{
	std::ofstream workingProxyFile("WorkingProxies.txt", std::ios::app);

	workingProxyFile << workingProxy << "\n";
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

		if(std::regex_match(ip, regexIp))
			ips.push_back(ip);


		if(std::regex_match(stringPort, regexPort))
			ports.push_back(std::stoi(stringPort));
	}
	return std::make_tuple(ips, ports);
}

void drawProxyWorkingGrid(std::string ipTested, int amountWorking, int amountTimedOut, int amountDead)
{
	system("cls");

	std::cout << "-- Working\n";
	std::cout << amountWorking;
	std::cout << "\n-- TimedOut\n";
	std::cout << amountTimedOut;
	std::cout << "\n-- Not Working\n";
	std::cout << amountDead;

	std::cout << "\n-- IPS TESTED\n";
	std::cout << ipTested << "\n";
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
	const int threadsCount{ 3 };
	std::thread tid[threadsCount];
	curl_global_init(CURL_GLOBAL_ALL);
	int ipI = 0;
	do {
		// ipI < ipsVector.size() prevents out of range error.
		for(int i = 0; i < threadsCount && ipI < ipsVector.size(); i++, ++ipI) {
			tid[i] = std::thread(&ProxyTester::checkProxyConnection, &test, ipsVector[ipI], portVector[ipI]);

			//printf("Thread: %d, used. ipI is %d\n", i, ipI);
		}

		//	///* now wait for all threads to terminate*/
		for(int i = 0; i < threadsCount; i++) {
			if(tid[i].joinable())
				tid[i].join();

			drawProxyWorkingGrid("test", test.getWorkingProxyStat(), test.getTimedOutProxyStat(), test.getDeadProxyStat());
		}

	} while(ipI < ipsVector.size() - 1);

	std::cout << "time elpased: " << r.elapsed() << "\n";
	std::cout << "Finished..... Press a letter to exit : ";
	std::cin.get();
}

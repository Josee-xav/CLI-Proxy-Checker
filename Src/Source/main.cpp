#include <iostream>
#include <vector>
#include <fstream>
#include <string>

#define CURL_STATICLIB
#include <curl.h>

/* proxyChecker couple of common responses that the function returns and its meannings.
	 (ip) : (port) worked! <- worked fine
	 (ip) : (port) Failed Reason : Couldn't connect to server. <- Failed to connect to host or proxy.
	 (ip) : (port) Failed Reason : Failure when receiving data from the peer <- Failure with receiving network data.
	 (ip) : (port) Failed TimedOut After 2 attempts of connecting.... <- it means that either the proxy is slow or unstable so the connection timed out. it may work but i wouldnt recomended using the proxy due to it probably being unstable..

*/

constexpr int AMOUNT_RETRY{ 2 };
std::string buffer; // string used by writer function to save cURL

/*
* This function gets called by libcurl as soon as there is data received that needs to be saved.
* The size of the data pointed to by char* data is size multiplied with size_t nmemb, it will not be zero terminated.
* Return the number of bytes actually taken care of.
* If that amount differs from the amount passed to your function, it'll signal an error to the library.
* This will abort the transferand return CURLE_WRITE_ERROR.
*/
static int writer(char* data, size_t size, size_t nmemb, std::string* buffer) {
	unsigned int result = 0;
	if (buffer != NULL) {
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

/*checkProxyConnection
	This function checks the connection of the proxy.

	If the connection times out on the first loop then on the 2nd loop it will try again if it still times out then it will return ip + " : " + std::to_string(port) + " Failed TimedOut After 2 attempts of connecting...."
*/
std::string checkProxyConnection(std::string ip, int port, std::string proxytype) {

	for (int i = 0; i < AMOUNT_RETRY; i++) {

		CURL* curlHandle;
		CURLcode curlResponse;
		curlHandle = curl_easy_init();

		if (curlHandle) {
			curl_easy_setopt(curlHandle, CURLOPT_URL, "https://www.google.com");
			curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);
			curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 15L);
			curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:69.0) Gecko/20100101 Firefox/69.0");

			// sets the proxyType
			if (proxytype == "http")
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
			else if (proxytype == "https")
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
			else if (proxytype == "socks4")
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
			else if (proxytype == "socks4a")
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
			else if (proxytype == "socks5")
				curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
			else
				return "Didnt select proper proxytype. type one of these :: http, https, socks4, socks4a, socks5";

			curl_easy_setopt(curlHandle, CURLOPT_PROXY, ip.c_str());
			curl_easy_setopt(curlHandle, CURLOPT_PROXYPORT, port);
			curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writer);
			curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &buffer);

			curlResponse = curl_easy_perform(curlHandle); // will perform the transfer as described in all of the options above
			curl_easy_cleanup(curlHandle);


			if (curlResponse != CURLE_OPERATION_TIMEDOUT) {
				if (curlResponse == CURLE_OK) {

					if (sizeof(buffer) > 0) {// receviced data thats greater than 0. meaning that the connection went though
						return  ip + " : " + std::to_string(port) + " worked!";
					}
				}
				else {
					return ip + " : " + std::to_string(port) + " Failed Reason : " + static_cast<std::string>(curl_easy_strerror(curlResponse));
				}
			}
		}
		else {
			return "The Curl handle Failed!";
		}

	}
	return ip + " : " + std::to_string(port) + " Failed TimedOut After 2 attempts of connecting...."; // retry didnt work
}

/* readTextFile
	This function uses a textfile name provided by the user thats stored in p_fileName.
	The fileName will be used so it can read the IP's and the port of that ip and then puts it puts that data into one of the vectors so the IPS will go to
	string vector "p_ips" and the ports will go to a int vector called "p_port"

	Variables are:
	p_fileName is the user fileName.
	p_amountOfIps is a referenced variable that counts the amount of ips they are.
	p_ips is the storage for all of the ips(not including ports).
	p_port is the storage for all of the ports.

*/
void readTextFile(std::string p_fileName, int& p_amountOfIps, std::vector<std::string>& p_ips, std::vector<int>& p_ports) {

	std::ifstream rFile(p_fileName);

	// If we couldn't open the output file stream for reading
	if (!rFile) {
		std::cout << "Uh oh, File: " << p_fileName << " could not be opened for reading!" << std::endl;
		Sleep(2000);
		exit(1);
	}

	bool foundColon = false;
	std::string bufferIP{};
	std::string bufferPort{};
	char c;
	while (rFile.get(c))
	{

		if (c != '\n') {

			if (c == ':') {
				foundColon = true;
			}
			else if (isdigit(c) || c == '.') { // checks if the character that its trying to input into the buffers is a number or is '.'
				if (foundColon != true) {
					bufferIP += c;
				}
				else {
					bufferPort += c;
				}
			}
			else {
				std::cerr << "Error File fomatting aint right. To fix this\n";
				std::cerr << "You need to make sure each line has formatting like this example: 1.1.1.1.1:6666\n";
				Sleep(2000);
				exit(1);
			}

		}
		else {
			int portt;
			try {
				portt = std::stoi(bufferPort); // converts the string that contains the port
			}
			catch (std::invalid_argument) { // this exception may happen if you have formatted ur file wrong. like an empty line in ur txt file
				std::cerr << "You probably have a empty line in your proxy list file. please remove it before using this.\n";
				Sleep(2000);
				exit(1);
			}

			p_ips.push_back(bufferIP); // pushes the found IP into the vector

			p_ports.push_back(portt); // pushes the found PORT into the vecotr

			p_amountOfIps++;

			bufferIP = {};
			bufferPort = {};
			foundColon = false;
		}

	}
	rFile.close();
}

int main() {
	std::vector<std::string> ipsVector;
	std::vector<int> portVector;
	int amountOfIps = 0;

	std::cout << "Whats the file name that contains all of the ips. example: proxyList.txt\n";
	std::string fileName{};
	std::getline(std::cin, fileName);

	std::cout << "Whats the type the protocol that all of the proxies are. The protocols are: http, https, socks4, socks4a, socks5\n";
	std::string proxiesProtocol{};
	std::getline(std::cin, proxiesProtocol);

	std::cout << "---------------------------------------------------------------------------------------------------\n";// line breaker

	readTextFile(fileName, amountOfIps, ipsVector, portVector);

	std::cout << "Found " << amountOfIps << " ips\n";
	for (int i = 0; i < amountOfIps; i++) {
		std::cout << checkProxyConnection(ipsVector.at(i), portVector.at(i), proxiesProtocol) << "\n";
	}
}

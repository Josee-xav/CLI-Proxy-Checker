#include "ProxyTester.h"
#include <mutex>

ProxyTester::ProxyTester(std::string protocol) :workingProxies(0), timedOutProxies(0), deadProxies(0), proxyProtocol(protocol)
{
}
std::mutex g_i_mutex;

void ProxyTester::checkProxyConnection(std::string ip, int port)
{
	std::string buffer; // string used by writer function to save cURL
	CURL* curlHandle;
	CURLcode curlResponse;
	curlHandle = curl_easy_init();

	if(curlHandle) {
		curl_easy_setopt(curlHandle, CURLOPT_URL, "https://www.google.com");
		curl_easy_setopt(curlHandle, CURLOPT_FOLLOWLOCATION, true);
		curl_easy_setopt(curlHandle, CURLOPT_CONNECT_ONLY, 1L);
		//curl_easy_setopt(curlHandle, CURLOPT_RESOLVE, host); // implement this to remove the dns resolving 
		curl_easy_setopt(curlHandle, CURLOPT_ACCEPT_ENCODING, "");
		curl_easy_setopt(curlHandle, CURLOPT_TIMEOUT, 5L);
		curl_easy_setopt(curlHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:69.0) Gecko/20100101 Firefox/69.0");

		// sets the proxyType
		if(proxyProtocol == "http")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
		else if(proxyProtocol == "https")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_HTTPS);
		else if(proxyProtocol == "socks4")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
		else if(proxyProtocol == "socks5")
			curl_easy_setopt(curlHandle, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);

		curl_easy_setopt(curlHandle, CURLOPT_PROXY, ip.c_str());
		curl_easy_setopt(curlHandle, CURLOPT_PROXYPORT, port);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, writer);
		curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, &buffer);

		curlResponse = curl_easy_perform(curlHandle); // will perform the transfer as described in all of the options above

		if(curlResponse != CURLE_OPERATION_TIMEDOUT) {
			if(curlResponse == CURLE_OK) {
				if(sizeof(buffer) > 0) {// receviced data thats greater than 0. meaning that the connection went though
					writeWorkingProxies(ip + " : " + std::to_string(port));

					printf("ip:%s,  %s", ip.c_str(), "worked\n");

					workingProxies++;
					testedIpsWithDetails.push_back(ip + " : " + std::to_string(port) + " worked!");
				}
			}
			else {
				printf("ip:%s,  %s", ip.c_str(), "dead\n");
				{
					std::lock_guard<std::mutex> guard(g_i_mutex);
					deadProxies++;
					testedIpsWithDetails.push_back(ip + " : " + std::to_string(port) + " Failed Reason : " + static_cast<std::string>(curl_easy_strerror(curlResponse)));
				}
			}
		}
		else {
			printf("ip:%s,  %s", ip.c_str(), "timedoutpr\n");
			{
				std::lock_guard<std::mutex> guard(g_i_mutex);
				timedOutProxies++;
				testedIpsWithDetails.push_back(ip + " : " + std::to_string(port) + " Failed Reason: Connection Timed out");
			}
		}
		curl_easy_cleanup(curlHandle);
	}
	else {
		std::cout << "The Curl handle Failed! Press a letter to exit\n"; // error
		//std::cin.get();
		exit(1);
	}
}

std::string ProxyTester::getTestedIpDetails(int pos)
{
	return testedIpsWithDetails.at(pos);
}

int ProxyTester::getWorkingProxyStat()
{
	return workingProxies;
}

int ProxyTester::getDeadProxyStat()
{
	return deadProxies;
}

int ProxyTester::getTimedOutProxyStat()
{
	return timedOutProxies;
}

int ProxyTester::writer(char* data, size_t size, size_t nmemb, std::string* buffer)
{
	unsigned int result = 0;
	if(buffer != NULL) {
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

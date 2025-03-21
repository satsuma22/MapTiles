#include "OSMDataLoader.h"

// Define a macro that includes the correct sleep function for the current platform
#ifdef _WIN32
#include <Windows.h>
#define Sleep(x) Sleep(x)
#else
#include <unistd.h>
#define Sleep(x) usleep(x * 1000)
#endif

OSMDataLoader::OSMDataLoader(double latMin, double lonMin, double latMax, double lonMax)
	: m_latMin(latMin), m_lonMin(lonMin), m_latMax(latMax), m_lonMax(lonMax)
{
	m_url = "http://overpass-api.de";
	m_query = "";
}

// makes the API request to the overpass-api and waits for a valid response
void OSMDataLoader::FetchOSMWays()
{
	m_query.clear();
	m_query.append("/api/interpreter?data=");

	// Construct the query
	m_query.append("(way[\"building\"](");
	m_query.append(std::to_string(m_latMin));
	m_query.append(",");
	m_query.append(std::to_string(m_lonMin));
	m_query.append(",");
	m_query.append(std::to_string(m_latMax));
	m_query.append(",");
	m_query.append(std::to_string(m_lonMax));
	m_query.append(");\n");
	m_query.append(">;\n");
	m_query.append(");\n");
	//m_query.append("out meta;");
	m_query.append("out;");

	httplib::Client client(m_url);

	m_result = client.Get(m_query);

	int attempts = 0;

	// sometimes the API request results in an error 
	while ((int)m_result.error() || GetHTTPStatus() != 200 && attempts < 3)
	{
		// Avoid making too many requests at the same time
		Sleep(200);
		m_result = client.Get(m_query);
		attempts++;
	}
}

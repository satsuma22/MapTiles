#include "OSMRasterTileLoader.h"

#include "utils.h"

// Define a macro that includes the correct sleep function for the current platform
#ifdef _WIN32
#include <Windows.h>
#define Sleep(x) Sleep(x)
#else
#include <unistd.h>
#define Sleep(x) usleep(x * 1000)
#endif

OSMRasterTileLoader::OSMRasterTileLoader(int zoom, double lat, double lon)
	: m_zoom(zoom), m_x(0), m_y(0)
{
	m_y = lat2tiley(lat, m_zoom);
	m_x = long2tilex(lon, m_zoom);

	m_url = "http://tile.openstreetmap.org";

	m_query = "/";
	m_query.append(std::to_string(zoom));
	m_query.append("/");
	m_query.append(std::to_string(m_x));
	m_query.append("/");
	m_query.append(std::to_string(m_y));
	m_query.append(".png");
}

OSMRasterTileLoader::OSMRasterTileLoader(int zoom, int x, int y)
	: m_zoom(zoom), m_x(x), m_y(y)
{
	m_url = "http://tile.openstreetmap.org";

	m_query = "/";
	m_query.append(std::to_string(zoom));
	m_query.append("/");
	m_query.append(std::to_string(m_x));
	m_query.append("/");
	m_query.append(std::to_string(m_y));
	m_query.append(".png");
}

// make the request to the tile server and wait for a valid response
void OSMRasterTileLoader::FetchTile()
{
	httplib::Client client(m_url);
	m_result = client.Get(m_query);

	int attempts = 0;

	while ((int)m_result.error() || GetHTTPStatus() != 200 && attempts < 10)
	{
		// Avoid making too many requests at the same time
		Sleep(50);
		m_result = client.Get(m_query);
		attempts++;
	}
}

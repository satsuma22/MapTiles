#pragma once

#include <string>
#include "httplib.h"

// Class to construct a request for and fetch a Raster Tile at a given location and zoom level from OSM tile server
class OSMRasterTileLoader
{
public:
	OSMRasterTileLoader(int zoom, double lat, double lon);
	OSMRasterTileLoader(int zoom, int x, int y);

	void FetchTile();
	std::string& GetResponse() { return m_result->body; };	// return the API response in the form of a string

	int GetErrorStatus() { return (int)m_result.error(); }
	int GetHTTPStatus() { return m_result->status; }

	int GetX() { return m_x; }
	int GetY() { return m_y; }

private:
	std::string m_url;
	std::string m_query;

	// Slippy tile map parameters
	// See: https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
	int m_zoom;
	int m_x;
	int m_y;

	httplib::Result m_result;
};
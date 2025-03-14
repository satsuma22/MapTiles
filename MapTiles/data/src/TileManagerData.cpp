#include "TileManagerData.h"

#include "OSMRasterTileLoader.h"
#include "OSMRasterTileProcessor.h"
#include "OSMDataLoader.h"
#include "OSMDataProcessor.h"

#include "utils.h"

#include "Timer.h"

Tile3DData TileManagerData::EmptyTile3D = Tile3DData(0, 0, 0, 0, std::vector<glm::dvec3>());
RasterTileData TileManagerData::EmptyRasterTile = RasterTileData(nullptr, 0, 0, 0, 0, 0);

TileManagerData::TileManagerData() : m_config(nullptr)
{
	EmptyTile3D.valid = false;
	EmptyRasterTile.valid = false;
}

TileManagerData::~TileManagerData()
{
	m_config = nullptr;
}

void TileManagerData::Init(GlobalConfig* config)
{
	m_config = config;
}

RasterTileData& TileManagerData::GetRasterTile(int zoom, int x, int y)
{
	RasterTileIndex index = { zoom, x, y };
	
	{
		std::lock_guard<std::mutex> lockCache(m_MutexRasterTiles);
		if (m_RasterTileCache.find(index) != m_RasterTileCache.end())
		{
			return m_RasterTileCache.at(index);
		}
	}

	OSMRasterTileLoader loader(zoom, x, y);
	loader.FetchTile();

	if (loader.GetErrorStatus() != 0 || loader.GetHTTPStatus() != 200)
	{
		std::cout << "[Tile Manager] Error fetching Raster Tile\n";
		return EmptyRasterTile;
	}

	OSMRasterTileProcessor processor(loader);
	RasterTileData rasterTile(processor.GetImage(), processor.GetHeight(), processor.GetWidth(), zoom, loader.GetX(), loader.GetY());

	std::lock_guard<std::mutex> lockActive(m_MutexRasterTiles);
	m_RasterTileCache.emplace(index, std::move(rasterTile));

	return m_RasterTileCache.at(index);
}

RasterTileData& TileManagerData::GetRasterTile(int zoom, double lat, double lon)
{
	int x = long2tilex(lon, zoom);
	int y = lat2tiley(lat, zoom);

	return GetRasterTile(zoom, x, y);
}

Tile3DData& TileManagerData::GetTile3D(double lat, double lon)
{
	Tile3DIndex index = { lat, lon };
	
	{
		std::lock_guard<std::mutex> lockCache(m_MutexTile3Ds);
		if (m_Tile3DCache.find(index) != m_Tile3DCache.end())
		{
			return m_Tile3DCache.at(index);
		}
	}

	double size = m_config->Tile3DSize;
	OSMDataLoader loader(lat, lon, lat + size, lon + size);
	loader.FetchOSMWays();

	if (loader.GetErrorStatus() != 0 || loader.GetHTTPStatus() != 200)
	{
		std::cout << "[Tile Manager] Error fetching OSM data\n";
		return EmptyTile3D;
	}

	OSMDataProcessor processor(loader);
	Tile3DData tile3DData(lat, lon, lat + size, lon + size, std::move(processor.GetTileGeometry()));

	// Do the coordinate conversion here
	{
		tile3DData.ConvertTo3DCoordinates(m_config->ReferencePoint.lat, m_config->ReferencePoint.lon);
	}

	std::lock_guard<std::mutex> lockActive(m_MutexTile3Ds);
	m_Tile3DCache.emplace(index, std::move(tile3DData));
	std::cout << "[Tile Manager] Tile 3D Cache Size: " << m_Tile3DCache.size() << "\n";

	return m_Tile3DCache.at(index);
}

void TileManagerData::ClearCache()
{
	std::lock_guard<std::mutex> lockRasterTiles(m_MutexRasterTiles);
	m_RasterTileCache.clear();

	std::lock_guard<std::mutex> lockTile3D(m_MutexTile3Ds);
	m_Tile3DCache.clear();
}

void TileManagerData::TrimCache()
{
	std::set<RasterTileIndex> to_be_removed_raster_tile;
	std::set<Tile3DIndex> to_be_removed_tile3D;

	for (const auto& element : m_RasterTileCache) {
		// Get the position of the tile in degrees
		double lat = tiley2lat(element.first.y, element.first.zoom);
		double lon = tilex2long(element.first.x, element.first.zoom);
		if (std::abs(lat - m_config->ReferencePoint.lat) > m_config->RasterTileRenderCacheTrimDistance ||
			std::abs(lon - m_config->ReferencePoint.lon) > m_config->RasterTileRenderCacheTrimDistance)
			to_be_removed_raster_tile.insert(element.first);
	}

	for (const auto& element : m_Tile3DCache) {
		// Get the position of the tile in degrees
		double lat = element.first.lat;
		double lon = element.first.lon;
		if (std::abs(lat - m_config->ReferencePoint.lat) > m_config->Tile3DRenderCacheTrimDistance ||
			std::abs(lon - m_config->ReferencePoint.lon) > m_config->Tile3DRenderCacheTrimDistance)
			to_be_removed_tile3D.insert(element.first);
	}

	std::lock_guard<std::mutex> lockRasterTiles(m_MutexRasterTiles);
	for (const auto& index : to_be_removed_raster_tile)
		m_RasterTileCache.erase(index);

	std::lock_guard<std::mutex> lockTile3D(m_MutexTile3Ds);
	for (const auto& index : to_be_removed_tile3D)
		m_Tile3DCache.erase(index);
}


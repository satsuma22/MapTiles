#include "TileManagerData.h"

#include "OSMRasterTileLoader.h"
#include "OSMRasterTileProcessor.h"
#include "OSMDataLoader.h"
#include "OSMDataProcessor.h"

#include "utils.h"

#include "Timer.h"

TileManagerData::TileManagerData() : m_config(nullptr)
{
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
		//Timer t("GetRasterTile [Cached]");

		if (m_RasterTileCache.find(index) != m_RasterTileCache.end())
		{
			//return m_RasterTileCache[index];
			return m_RasterTileCache.at(index);
		}
		// If we don't return from the function in the if statement above, the tile in not in the cache
		//t.Cancel();
	}

	//Timer t("t");
	OSMRasterTileLoader loader(zoom, x, y);
	loader.FetchTile();
	//t.~Timer();

	//Timer t("l");
	OSMRasterTileProcessor processor(loader);
	RasterTileData rasterTile(processor.GetImage(), processor.GetHeight(), processor.GetWidth(), zoom, loader.GetX(), loader.GetY());
	//t.~Timer();

	std::lock_guard<std::mutex> lockActive(m_MutexRasterTiles);
	//m_RasterTileCache[index] = std::move(rasterTile);
	m_RasterTileCache.emplace(index, std::move(rasterTile));

	//return m_RasterTileCache[index];
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
		//Timer t("GetTile3D [Cached]");

		if (m_Tile3DCache.find(index) != m_Tile3DCache.end())
		{
			//return m_Tile3DCache[index];
			return m_Tile3DCache.at(index);
		}
		//t.Cancel();
	}

	//Timer t("s");
	double size = m_config->Tile3DSize;
	OSMDataLoader loader(lat, lon, lat + size, lon + size);
	loader.FetchOSMWays();
	//t.~Timer();

	//Timer t("T");
	OSMDataProcessor processor(loader);
	Tile3DData tile3DData(lat, lon, lat + size, lon + size, processor.GetTileGeometry());
	//t.~Timer();

	// Do the coordinate conversion here
	{
		//Timer t1("Coordinate Conversion");
	tile3DData.ConvertTo3DCoordinates(m_config->ReferencePoint.lat, m_config->ReferencePoint.lon);
	}

	std::lock_guard<std::mutex> lockActive(m_MutexTile3Ds);
	//m_Tile3DCache[index] = tile3DData;
	m_Tile3DCache.emplace(index, std::move(tile3DData));
	std::cout << "[Tile Manager] Tile 3D Cache Size: " << m_Tile3DCache.size() << "\n";

	//return m_Tile3DCache[index];
	return m_Tile3DCache.at(index);
}


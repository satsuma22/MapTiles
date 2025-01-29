#include "TileManager.h"
#include "utils.h"
#include "WGS84toCartesian.hpp"

#include <iostream>

#include <thread>

TileManager::TileManager() : cam_lat(0), cam_lon(0), altitude(0), config(nullptr),
frustum_min_lat(0), frustum_max_lat(0), frustum_min_lon(0),
frustum_max_lon(0), min_extent({ 0, 0 }), max_extent({ 0, 0 }), frustum_bbox_min(0), frustum_bbox_max(0)
{
}

TileManager::~TileManager()
{
	config = nullptr;
}

void TileManager::Init(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _altitude;
	config = _conf;
	tile_manager_data.Init(config);
}

void TileManager::ReInit(double _lat, double _lon, double _altitude, GlobalConfig* _conf)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _altitude;
	config = _conf;

	active_raster_tile.clear();
	active_tile3D.clear();
}

void TileManager::Finalize()
{
	// active_raster_tiles.clear();
	// neighbour_set_raster_tile.clear();
}

std::map<Tile3DIndex, Tile3DRender>& TileManager::GetActiveTile3Ds()
{
	return active_tile3D;
}

std::map<RasterTileIndex, RasterTileRender>& TileManager::GetActiveRasterTiles()
{
	return active_raster_tile;
}

void TileManager::Update()
{
	AddRasterTiles();
	AddTile3D();

	min_extent = { 90, 180 };
	max_extent = { -90, -180 };

	if (config->FrustumBasedTileGeneration)
	{
		GenerateRasterTileFrustumNeighbours();
		GenerateTile3DFrustumNeighbours();
	}
	else
	{
		GenerateRasterTileNeighbours();
		GenerateTile3DNeighbours();
	}

	RemoveRasterTiles();
	RemoveTile3Ds();
	PruneNeighbourSetRasterTile();
	PruneNeighbourSetTile3D();
	GetRasterTileNeighbours();
	GetTile3DNeighbours();
}

void TileManager::SetPosition(double _lat, double _lon, double _alt)
{
	cam_lat = _lat;
	cam_lon = _lon;
	altitude = _alt;
}

std::pair<std::array<double, 2>, std::array<double, 2>> TileManager::GetExtent()
{
	return std::pair<std::array<double, 2>, std::array<double, 2>>(min_extent, max_extent);
}

void TileManager::GenerateRasterTileFrustumNeighbours()
{
	// clear the neighbour set so that we don't request outdated tiles
	neighbour_set_raster_tile.clear();

	int zoom = (int)GetZoom();

	int camX = long2tilex(cam_lon, zoom);
	int camY = lat2tiley(cam_lat, zoom);

	int minX = long2tilex(frustum_min_lon, zoom);
	int maxY = lat2tiley(frustum_min_lat, zoom);
	int maxX = long2tilex(frustum_max_lon, zoom);
	int minY = lat2tiley(frustum_max_lat, zoom);

	// Number of tiles at the current zoom level
	int nTiles = std::exp2(zoom);

	int count = 0;

	for (int i = minX; i <= maxX; i++) {
		// check if we are out of bounds
		if (i < 0 || i >= nTiles || std::abs(i - camX) > config->FrustumRasterTilesCount)
			continue;
		for (int j = minY; j <= maxY; j++) {
			// check if we are out of bounds
			if (j < 0 || j >= nTiles || std::abs(j - camY) > config->FrustumRasterTilesCount)
				continue;

			bool isVisible = true;
			glm::vec3 tileMin(0), tileMax(0);

			double lat_min = tiley2lat(j + 1, zoom);
			double lat_max = tiley2lat(j, zoom);
			double lon_min = tilex2long(i, zoom);
			double lon_max = tilex2long(i + 1, zoom);

			std::array<double, 2> min =
				wgs84::toCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon }, { lat_min, lon_min });
			std::array<double, 2> max = wgs84::toCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon },
				{ lat_max, lon_max });

			tileMin[0] = min[0];
			tileMin[1] = 0.0f;
			tileMin[2] = -max[1];

			tileMax[0] = max[0];
			tileMax[1] = 1.0f;
			tileMax[2] = -min[1];

			for (int k = 0; k < 6; k++) {
				if (IsBoxCompletelyBehindPlane(tileMin, tileMax, frustum_planes[k])) {
					isVisible = false;
					break;
				}
			}

			if (isVisible) {
				if (lat_min < min_extent[0])
					min_extent[0] = lat_min;
				if (lat_max > max_extent[0])
					max_extent[0] = lat_max;
				if (lon_min < min_extent[1])
					min_extent[1] = lon_min;
				if (lon_max > max_extent[1])
					max_extent[1] = lon_max;

				RasterTileIndex index = { zoom, i, j };
				neighbour_set_raster_tile.insert(index);
				count++;
			}

		}
	}
}

void TileManager::GenerateTile3DFrustumNeighbours()
{
	// clear the neighbour set so that we don't request outdated tiles
	neighbour_set_tile3D.clear();

	double size = config->Tile3DSize;

	int count = 0;

	for (double lat = frustum_min_lat; lat <= frustum_max_lat; lat += size) {
		// check if we are out of bounds
		if (lat < -89 || lat >= 89 || std::abs(lat - cam_lat) > config->FrustumTile3DMaxDistance)
			continue;
		for (double lon = frustum_min_lon; lon <= frustum_max_lon; lon += size) {
			if (lon < -180 || lon >= 180 || std::abs(lon - cam_lon) > config->FrustumTile3DMaxDistance)
				continue;

			bool isVisible = true;
			glm::vec3 tileMin(0), tileMax(0);

			std::array<double, 2> min = wgs84::toCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon }, { lat, lon });
			std::array<double, 2> max = wgs84::toCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon }, { lat + size, lon + size });

			tileMin[0] = min[0];
			tileMin[1] = 0.0f;
			tileMin[2] = -max[1];

			tileMax[0] = max[0];
			tileMax[1] = 100.0f;
			tileMax[2] = -min[1];

			for (int i = 0; i < 6; i++)
			{
				if (IsBoxCompletelyBehindPlane(tileMin, tileMax, frustum_planes[i]))
				{
					isVisible = false;
					break;
				}
			}

			if (isVisible)
			{
				// hack to get around floating point precision issues for indexing tiles
				lat = (double)llround(lat * (1 / size)) * size;
				lon = (double)llround(lon * (1 / size)) * size;

				if (lat < min_extent[0])
					min_extent[0] = lat;
				if (lat + size > max_extent[0])
					max_extent[0] = lat + size;
				if (lon < min_extent[1])
					min_extent[1] = lon;
				if (lon + size > max_extent[1])
					max_extent[1] = lon + size;

				Tile3DIndex index = { lat, lon };
				neighbour_set_tile3D.insert(index);
				count++;
			}
		}
	}
}

void TileManager::GenerateRasterTileNeighbours()
{
	// clear the neighbour set so that we don't request outdated tiles
	neighbour_set_raster_tile.clear();

	int k = config->NeighbourhoodFetchSizeRasterTile;

	int centerX;
	int centerY;

	int zoom = (int)GetZoom();

	centerX = long2tilex(cam_lon, zoom);
	centerY = lat2tiley(cam_lat, zoom);
	// Number of tiles at the current zoom level
	int nTiles = std::exp2(zoom);

	for (int i = -k; i <= k; i++) {
		// check if we are out of bounds
		if ((i + centerX) < 0 || (i + centerX) >= nTiles)
			continue;
		for (int j = -k; j <= k; j++) {
			// check if we are out of bounds
			if ((j + centerY) < 0 || (j + centerY) >= nTiles)
				continue;


			auto pos_min = wgs84::fromCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon },
				{ tiley2lat(j + centerY, zoom), tilex2long(i + centerX, zoom) });
			auto pos_max = wgs84::fromCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon },
				{ tiley2lat(j + centerY + 1, zoom), tilex2long(i + centerX + 1, zoom) });

			if (pos_min[0] < min_extent[0])
				min_extent[0] = pos_min[0];
			if (pos_max[0] > max_extent[0])
				max_extent[0] = pos_max[0];
			if (pos_min[1] < min_extent[1])
				min_extent[1] = pos_min[1];
			if (pos_max[1] > max_extent[1])
				max_extent[1] = pos_max[1];

			RasterTileIndex index = { zoom, i + centerX, j + centerY };
			neighbour_set_raster_tile.insert(index);
		}
	}
}

void TileManager::GenerateTile3DNeighbours()
{
	// clear the neighbour set so that we don't request outdated tiles
	neighbour_set_tile3D.clear();

	int k = config->NeighbourhoodFetchSizeTile3D;
	double size = config->Tile3DSize;

	double centerLat = std::floor(cam_lat / size) * size;
	double centerLon = std::floor(cam_lon / size) * size;

	for (int i = -k; i <= k; i++) {
		// check if we are out of bounds
		if ((i * size + centerLat) < -89 || (i * size + centerLat) >= 89)
			continue;
		for (int j = -k; j <= k; j++) {
			if ((j * size + centerLon) < -180 || (j * size + centerLon) >= 180)
				continue;

			double _lat = i * size + centerLat;
			double _lon = j * size + centerLon;

			_lat = (double)llround(_lat * (1 / size)) * size;
			_lon = (double)llround(_lon * (1 / size)) * size;

			if (_lat < min_extent[0])
				min_extent[0] = _lat;
			if (_lat + size > max_extent[0])
				max_extent[0] = _lat + size;
			if (_lon < min_extent[1])
				min_extent[1] = _lon;
			if (_lon + size > max_extent[1])
				max_extent[1] = _lon + size;

			Tile3DIndex index = { _lat, _lon };
			neighbour_set_tile3D.insert(index);
		}
	}
}

void TileManager::RemoveRasterTiles()
{
	int size = GetActiveRasterTiles().size();

	// Create a vector that will store the indices that need to be removed
	std::vector<RasterTileIndex> indices;
	indices.resize(size);

	// Get the indices to be removed from the queue
	for (auto const& element : queue_raster_tiles) {
		if (neighbour_set_raster_tile.find(element.first) == neighbour_set_raster_tile.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the queue
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueRasterTiles);
	for (auto& index : indices) {
		queue_raster_tiles.erase(index);
	}

	// clear the indices
	indices.clear();

	// Get the indices to be removed from the background list
	for (auto const& element : active_raster_tile) {
		if (neighbour_set_raster_tile.find(element.first) == neighbour_set_raster_tile.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the background list
	std::lock_guard<std::mutex> lockActive(m_MutexActiveRasterTiles);
	for (auto& index : indices) {
		active_raster_tile.erase(index);
	}
}

void TileManager::RemoveTile3Ds()
{
	int size = GetActiveTile3Ds().size();

	// Create a vector that will store the indices that need to be removed
	std::vector<Tile3DIndex> indices;
	indices.resize(size);

	// Get the indices to be removed from the queue
	for (auto const& element : queue_tile3Ds) {
		if (neighbour_set_tile3D.find(element.first) == neighbour_set_tile3D.end()) {
			indices.push_back(element.first);
		}
	}

	// Remove Raster Tiles from the queue
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueTile3Ds);
	for (auto& index : indices) {
		queue_tile3Ds.erase(index);
	}

	// clear the indices
	indices.clear();

	// Get the indices to be removed from the background list
	for (auto const& element : active_tile3D) {
		if (neighbour_set_tile3D.find(element.first) == neighbour_set_tile3D.end()) {
			indices.push_back(element.first);
			//std::cout << "Removing Tile: (" << element.first.lat << ", " << element.first.lon << ")\n";
		}
	}

	// Remove Raster Tiles from the background list
	std::lock_guard<std::mutex> lockActive(m_MutexActiveTile3Ds);
	for (auto& index : indices) {
		active_tile3D.erase(index);
	}
}

void TileManager::PruneNeighbourSetRasterTile()
{
	std::vector<RasterTileIndex> indices;

	for (const auto& element : neighbour_set_raster_tile) {
		if (queue_raster_tiles.find(element) != queue_raster_tiles.end()) {
			indices.push_back(element);
		}
		else if (active_raster_tile.find(element) != active_raster_tile.end()) {
			indices.push_back(element);
		}
		else if (requested_raster_tile.find(element) != requested_raster_tile.end())
		{
			indices.push_back(element);
		}
	}

	// Remove already present tiles from the neighbour set
	for (auto& index : indices) {
		neighbour_set_raster_tile.erase(index);
	}
}

void TileManager::PruneNeighbourSetTile3D()
{
	std::vector<Tile3DIndex> indices;

	for (const auto& element : neighbour_set_tile3D) {
		if (queue_tile3Ds.find(element) != queue_tile3Ds.end()) {
			indices.push_back(element);
		}
		else if (active_tile3D.find(element) != active_tile3D.end()) {
			indices.push_back(element);
		}
		else if (requested_tile3D.find(element) != requested_tile3D.end()) {
			indices.push_back(element);
		}
	}

	// Remove already present tiles from the neighbour set
	for (auto& index : indices) {
		neighbour_set_tile3D.erase(index);
	}
}

void TileManager::GetRasterTileNeighbours()
{
	std::set<RasterTileIndex> to_be_removed;

	for (const RasterTileIndex& itr : neighbour_set_raster_tile) {
		const RasterTileIndex& index = itr;

		if (m_RasterTileCache.find(index) != m_RasterTileCache.end())
		{
			active_raster_tile[index] = m_RasterTileCache[index];
			to_be_removed.insert(index);
		}

		else if (requested_raster_tile.size() < config->MaxRasterTileRequestThreads)
		{
			std::thread t(&TileManager::AddRasterTileToQueue, this, index);
			t.detach();
			requested_raster_tile.insert(index);
			to_be_removed.insert(index);
		}
	}

	// Once the request is made, remove the index from the set
	for (const RasterTileIndex& index : to_be_removed) {
		neighbour_set_raster_tile.erase(index);
	}
}

void TileManager::GetTile3DNeighbours()
{
	std::set<Tile3DIndex> to_be_removed;

	for (const Tile3DIndex& itr : neighbour_set_tile3D) {
		const Tile3DIndex& index = itr;

		if (m_Tile3DCache.find(index) != m_Tile3DCache.end())
		{
			active_tile3D[index] = m_Tile3DCache[index];
			to_be_removed.insert(index);
		}

		else if (requested_tile3D.size() < config->MaxTile3DRequestThreads)
		{
			std::thread t(&TileManager::AddTile3DToQueue, this, index);
			t.detach();
			requested_tile3D.insert(index);
			to_be_removed.insert(index);
		}
	}

	// Once the request is made, remove the index from the set
	for (const Tile3DIndex& index : to_be_removed) {
		neighbour_set_tile3D.erase(index);
	}
}

void TileManager::AddRasterTileToQueue(RasterTileIndex index)
{
	RasterTileData& tileData = tile_manager_data.GetRasterTile(index.zoom, index.x, index.y);
	std::lock_guard<std::mutex> lock(m_MutexQueueRasterTiles);
	queue_raster_tiles.emplace(index, tileData);
	requested_raster_tile.erase(index);
}

void TileManager::AddTile3DToQueue(Tile3DIndex index)
{
	Tile3DData& tileData = tile_manager_data.GetTile3D(index.lat, index.lon);
	std::lock_guard<std::mutex> lock(m_MutexQueueTile3Ds);
	queue_tile3Ds.emplace(index, tileData);
	requested_tile3D.erase(index);
}

void TileManager::AddRasterTiles()
{
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueRasterTiles);
	if (queue_raster_tiles.empty()) {
		return;
	}

	std::lock_guard<std::mutex> lockActive(m_MutexActiveRasterTiles);
	while (!queue_raster_tiles.empty()) {
		auto& element = *(queue_raster_tiles.begin());
		RasterTileRender tile(element.second, config->ReferencePoint.lat, config->ReferencePoint.lon);

		active_raster_tile[element.first] = tile;
		m_RasterTileCache[element.first] = tile;

		queue_raster_tiles.erase(element.first);
	}
}

void TileManager::AddTile3D()
{
	std::lock_guard<std::mutex> lockQueue(m_MutexQueueTile3Ds);
	if (queue_tile3Ds.empty()) {
		return;
	}

	std::lock_guard<std::mutex> lockActive(m_MutexActiveTile3Ds);
	while (!queue_tile3Ds.empty()) {
		auto const& element = *(queue_tile3Ds.begin());
		Tile3DRender tile(element.second, config->ReferencePoint.lat, config->ReferencePoint.lon);

		active_tile3D[element.first] = tile;
		m_Tile3DCache[element.first] = tile;

		queue_tile3Ds.erase(element.first);
	}
}

void TileManager::ClearRenderCache()
{
	std::cout << "Clearing Render Cache\n";
	std::cout << "Raster Tile Cache Size: " << m_RasterTileCache.size() << std::endl;
	std::cout << "Tile3D Cache Size: " << m_Tile3DCache.size() << std::endl;
	m_RasterTileCache.clear();
	m_Tile3DCache.clear();
}

void TileManager::TrimRenderCache()
{
	std::set<RasterTileIndex> to_be_removed_raster_tile;
	std::set<Tile3DIndex> to_be_removed_tile3D;

	for (const auto& element : m_RasterTileCache)
	{
		// Get the position of the tile in degrees
		double lat = tiley2lat(element.first.y, element.first.zoom);
		double lon = tilex2long(element.first.x, element.first.zoom);

		if (std::abs(lat - cam_lat) > config->RasterTileRenderCacheTrimDistance ||
			std::abs(lon - cam_lon) > config->RasterTileRenderCacheTrimDistance)
			to_be_removed_raster_tile.insert(element.first);
	}

	for (const auto& element : m_Tile3DCache) {
		// Get the position of the tile in degrees
		double lat = element.first.lat;
		double lon = element.first.lon;
		if (std::abs(lat - cam_lat) > config->Tile3DRenderCacheTrimDistance ||
			std::abs(lon - cam_lon) > config->Tile3DRenderCacheTrimDistance)
			to_be_removed_tile3D.insert(element.first);
	}

	size_t raster_size_before = m_RasterTileCache.size();
	size_t tile3D_size_before = m_Tile3DCache.size();

	for (const auto& index : to_be_removed_raster_tile)
		m_RasterTileCache.erase(index);

	for (const auto& index : to_be_removed_tile3D)
		m_Tile3DCache.erase(index);

	size_t raster_size_after = m_RasterTileCache.size();
	size_t tile3D_size_after = m_Tile3DCache.size();

	std::cout << "Raster Tile Cache Size: " << raster_size_before << " -> " << raster_size_after << std::endl;
	std::cout << "Tile3D Cache Size: " << tile3D_size_before << " -> " << tile3D_size_after << std::endl;
}

bool TileManager::IsBoxCompletelyBehindPlane(const glm::vec3& boxMin, const glm::vec3& boxMax,
	const glm::vec4& plane)
{
	return glm::dot(plane, glm::vec4(boxMin.x, boxMin.y, boxMin.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMin.x, boxMin.y, boxMax.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMin.x, boxMax.y, boxMin.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMin.x, boxMax.y, boxMax.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMax.x, boxMin.y, boxMin.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMax.x, boxMin.y, boxMax.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMax.x, boxMax.y, boxMin.z, 1)) < 0 &&
		glm::dot(plane, glm::vec4(boxMax.x, boxMax.y, boxMin.z, 1)) < 0;
}

void TileManager::CalculateViewFrustum(const glm::mat4& mvp)
{
	/*
	frustum_planes[0] = (mvp[3] + mvp[0]);
	frustum_planes[1] = (mvp[3] - mvp[0]);
	frustum_planes[2] = (mvp[3] + mvp[1]);
	frustum_planes[3] = (mvp[3] - mvp[1]);
	frustum_planes[4] = (mvp[3] + mvp[2]);
	frustum_planes[5] = (mvp[3] - mvp[2]);
	*/

	glm::mat4 mvpt = glm::transpose(mvp);

	frustum_planes[0] = (mvpt[3] + mvpt[0]);
	frustum_planes[1] = (mvpt[3] - mvpt[0]);
	frustum_planes[2] = (mvpt[3] + mvpt[1]);
	frustum_planes[3] = (mvpt[3] - mvpt[1]);
	frustum_planes[4] = (mvpt[3] + mvpt[2]);
	frustum_planes[5] = (mvpt[3] - mvpt[2]);

	constexpr double double_min = std::numeric_limits<double>::min();
	constexpr double double_max = std::numeric_limits<double>::max();
	frustum_bbox_min = { double_max, double_max, double_max };
	frustum_bbox_max = { double_min, double_min, double_min };

	glm::mat4 invMvp = glm::inverse(mvp);

	for (int x = -1; x <= 1; x += 2)
		for (int y = -1; y <= 1; y += 2)
			for (int z = -1; z <= 1; z += 2)
			{
				glm::dvec4 corner = invMvp * glm::dvec4((double)x, (double)y, (double)z, 1);
				corner /= corner.w;
				if (corner[0] < frustum_bbox_min[0])
					frustum_bbox_min[0] = corner[0];
				if (corner[0] > frustum_bbox_max[0])
					frustum_bbox_max[0] = corner[0];
				if (corner[1] < frustum_bbox_min[1])
					frustum_bbox_min[1] = corner[1];
				if (corner[1] > frustum_bbox_max[1])
					frustum_bbox_max[1] = corner[1];
				if (corner[2] < frustum_bbox_min[2])
					frustum_bbox_min[2] = corner[2];
				if (corner[2] > frustum_bbox_max[2])
					frustum_bbox_max[2] = corner[2];
			}

	std::array<double, 2> frustum_min = wgs84::fromCartesian(
		{ config->ReferencePoint.lat, config->ReferencePoint.lon }, { frustum_bbox_min[0], -frustum_bbox_max[2] });

	std::array<double, 2> frustum_max = wgs84::fromCartesian({ config->ReferencePoint.lat, config->ReferencePoint.lon },
		{ frustum_bbox_max[0], -frustum_bbox_min[2] });

	double& size = config->Tile3DSize;
	frustum_min_lat = std::floor(frustum_min[0] / size) * size;
	frustum_min_lon = std::floor(frustum_min[1] / size) * size;
	frustum_max_lat = std::ceil(frustum_max[0] / size) * size;
	frustum_max_lon = std::ceil(frustum_max[1] / size) * size;

}


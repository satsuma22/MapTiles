#pragma once

#include "../../Config.h"
#include "TileManagerData.h"
#include "RasterTileData.h"
#include "opengl/RasterTileRender.h"
#include "Tile3DData.h"
#include "opengl/Tile3DRender.h"

#include <glm.hpp>

#include <vector>
#include <array>
#include <set>

class TileManager
{
public:
	TileManager();
	~TileManager();

	void Init(double _lat, double _lon, double altitude, GlobalConfig* conf);
	void ReInit(double _lat, double _lon, double altitude, GlobalConfig* conf);
	void Finalize();

	void Update();
	void SetPosition(double _lat, double _lon, double _alt);

	std::map<Tile3DIndex, Tile3DRender>& GetActiveTile3Ds();
	std::map<RasterTileIndex, RasterTileRender>& GetActiveRasterTiles();

	std::pair<std::array<double, 2>, std::array<double, 2>> GetExtent();

	void CalculateViewFrustum(const glm::mat4& mvp);
	bool IsBoxCompletelyBehindPlane(const glm::vec3& boxMin, const glm::vec3& boxMax,
		const glm::vec4& plane);

private:
	void GenerateRasterTileFrustumNeighbours();
	void GenerateTile3DFrustumNeighbours();
	void GenerateRasterTileNeighbours();
	void GenerateTile3DNeighbours();
	void RemoveRasterTiles();
	void RemoveTile3Ds();
	void PruneNeighbourSetRasterTile();
	void PruneNeighbourSetTile3D();
	void GetRasterTileNeighbours();
	void GetTile3DNeighbours();
	void AddRasterTileToQueue(RasterTileIndex index);
	void AddTile3DToQueue(Tile3DIndex index);
	void AddRasterTiles();
	void AddTile3D();

	inline float GetZoom() const
	{
		return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(cam_lat)) /
			(std::max(1.0, altitude)))); /*Circumference of earth = 40075016*/
	}
	inline float GetZoomAtAltitude(float alt) const
	{
		return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(cam_lat)) /
			(std::max(1.0f, alt)))); /*Circumference of earth = 40075016*/
	}

private:
	double cam_lat, cam_lon, altitude;
	GlobalConfig* config;
	TileManagerData tile_manager_data;

	std::set<RasterTileIndex> neighbour_set_raster_tile;
	std::set<Tile3DIndex> neighbour_set_tile3D;
	std::set<RasterTileIndex> requested_raster_tile;
	std::set<Tile3DIndex> requested_tile3D;
	std::map<RasterTileIndex, RasterTileRender> active_raster_tile;
	std::map<Tile3DIndex, Tile3DRender> active_tile3D;
	std::map<RasterTileIndex, RasterTileData> queue_raster_tiles;
	std::map<Tile3DIndex, Tile3DData> queue_tile3Ds;

	mutable std::mutex m_MutexActiveRasterTiles;
	mutable std::mutex m_MutexActiveTile3Ds;

	mutable std::mutex m_MutexRequestedRasterTiles;
	mutable std::mutex m_MutexRequestTile3Ds;

	mutable std::mutex m_MutexQueueRasterTiles;
	mutable std::mutex m_MutexQueueTile3Ds;

	std::array<glm::dvec4, 6> frustum_planes;
	glm::dvec3 frustum_bbox_min, frustum_bbox_max;
	double frustum_min_lat, frustum_max_lat, frustum_min_lon, frustum_max_lon;
	std::array<double, 2> min_extent, max_extent;
};
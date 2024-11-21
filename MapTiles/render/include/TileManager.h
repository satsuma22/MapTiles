#pragma once

#include <vector>
#include <set>
#include <map>
#include <chrono>

#include <opengl/Shader.h>
#include <opengl/Texture.h>
#include <opengl/VertexArray.h>
#include <opengl/VertexBuffer.h>
#include <opengl/RasterTileRender.h>
#include <opengl/Tile3DRender.h>

#include "TileManagerData.h"
#include "utils.h"
#include "../../Config.h"

#include <mutex>

// Class to manage the creation and deletion of tiles based on the current specified location
class TileManager
{
public:
	TileManager();
	void Init(double lat, double lon, int altitude, GlobalConfig& config);
	void Finalize();
	
	void SetPosition(double lat, double lon, double altitude);

	// Performs one update of the TileManager. It consists of the following:
	// 1. Add 1 tile from the queue for each tile type
	// 2. Checks if the last update was finished or if enough time has passed. If not, returns. Else it continues to the next step
	// 3. Swaps the active and background tiles
	// 4. Generates a neighbourhood set based on the current location. The number of neighbours is specified in the GlobalConfig object
	// 5. Remove tiles from the active set and the queue that are not in the current neighbourhood set
	// 6. Removes tiles from the neighbour set that are already in the active set and the queue
	// 7. Generates an aysnchronous request to create a tile for every tile still remaining in the neighbour set using std::threads
	void Update();


	std::map<RasterTileIndex, RasterTileRender>& GetActiveRasterTiles() { return m_ActiveRasterTiles; }
	std::map<Tile3DIndex, Tile3DRender>& GetActiveTiles3D() { return  m_ActiveTile3Ds; }

private:
	bool AllActiveTilesProcessed();
	bool AllBackgroundTilesProcessed();
	bool AllQueuesEmpty();

	void AddRasterTileToQueue(RasterTileIndex index);
	void AddTile3DToQueue(Tile3DIndex index);

	// Generates the neighbourhood set for Raster tiles based on the current location and the size of the Raster tile neighbourhood
	void GenerateRasterTileNeighbours();
	// Generates the neighbourhood set for Tile 3Ds based on the current location and the size of the Tile3D neighbourhood
	void GenerateTile3DNeighbours();

	// Generates asynchronous requests for all the Raster tiles in the neighbour set
	void GetRasterTileNeighbours();
	// Generates asynchronous requests for all the Tile3Ds in the neighbour set
	void GetTile3DNeighbours();

	// Remove all the Raster Tiles from the active Raster Tiles and the Raster Tile queue that are not in the current Neighbourhood
	void RemoveRasterTiles();
	// Remove all the Tile3Ds from the active Tile3Ds and the Tile3Ds queue that are not in the current Neighbourhood
	void RemoveTile3Ds();

	// Remove the raster tiles from neighbour set that are already active or in the queue
	void PruneNeighbourSetRasterTile();
	// Remove the tile3Ds from neighbour set that are already active or in the queue
	void PruneNeighbourSetTile3D();

	// Adds the tile on the top of the Raster Tile queue to the active set of Raster Tiles
	void AddRasterTileFromQueue();
	// Adds the tile on the top of the Tile3D queue to the active set of Tile3Ds
	void AddTile3DFromQueue();

	inline float GetZoom() const { return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(m_lat)) / (std::max(1.0, m_altitude)))); /*Circumference of earth = 40075016*/ }
	inline float GetZoomAtAltitude(float altitude)  const { return std::min(19.0f, std::log2f(40075016 * std::cos(glm::radians(m_lat)) / (std::max(1.0f, altitude)))); /*Circumference of earth = 40075016*/ }

private:
	double m_lat, m_lon, m_altitude;

	bool m_moved;
	bool m_lastUpdateFinished;
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastUpdateTimePoint;
	
	GlobalConfig m_config;

	std::map<RasterTileIndex, RasterTileRender> m_ActiveRasterTiles;
	std::map<Tile3DIndex, Tile3DRender> m_ActiveTile3Ds;

	std::map<RasterTileIndex, RasterTileRender> m_BackgroundRasterTiles;
	std::map<Tile3DIndex, Tile3DRender> m_BackgroundTile3Ds;

	std::map<RasterTileIndex, RasterTileData> m_QueueRasterTiles;
	std::map<Tile3DIndex, Tile3DData> m_QueueTile3Ds;

	std::set<RasterTileIndex> m_NeighbourSetRasterTiles;
	std::set<Tile3DIndex> m_NeighbourSetTile3Ds;

	TileManagerData m_tileManagerData;
	
	mutable std::mutex m_MutexActiveRasterTiles;
	mutable std::mutex m_MutexActiveTile3Ds;

	mutable std::mutex m_MutexNeighbourSetRasterTiles;
	mutable std::mutex m_MutexNeighbourSetTile3Ds;

	mutable std::mutex m_MutexQueueRasterTiles;
	mutable std::mutex m_MutexQueueTile3Ds;
};
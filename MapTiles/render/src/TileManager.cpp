#include "TileManager.h"

#include "OSMRasterTileLoader.h"
#include "OSMRasterTileProcessor.h"
#include "RasterTileData.h"

#include "opengl/RasterTileRender.h"

#include "OSMDataLoader.h"
#include "OSMDataProcessor.h"
#include "Tile3DData.h"

#include "opengl/Tile3DRender.h"

#include "../../Timer.h"

#include <thread>

TileManager::TileManager()
    : m_lat(0), m_lon(0), m_altitude(0), m_moved(false), m_lastUpdateFinished(true)
{
}

void TileManager::Init(double lat, double lon, int altitude, GlobalConfig& config)
{
    m_config = config;
    m_tileManagerData.Init(config);

    SetPosition(lat, lon, altitude);
}

void TileManager::Finalize()
{
    m_ActiveRasterTiles.clear();
    m_ActiveTile3Ds.clear();
}

bool TileManager::AllActiveTilesProcessed()
{
    int rasterTilesGridSize = m_config.NeighbourhoodFetchSizeRasterTile * 2 + 1;
    int tile3DGridSize = m_config.NeighbourhoodFetchSizeTile3D * 2 + 1;

    if (m_ActiveRasterTiles.size() == rasterTilesGridSize * rasterTilesGridSize && m_ActiveTile3Ds.size() == tile3DGridSize * tile3DGridSize)
        return true;
    
    return false;
}

bool TileManager::AllBackgroundTilesProcessed()
{
    int rasterTilesGridSize = m_config.NeighbourhoodFetchSizeRasterTile * 2 + 1;
    int tile3DGridSize = m_config.NeighbourhoodFetchSizeTile3D * 2 + 1;

    if (m_BackgroundRasterTiles.size() == rasterTilesGridSize * rasterTilesGridSize && m_BackgroundTile3Ds.size() == tile3DGridSize * tile3DGridSize)
        return true;

    return false;
}

bool TileManager::AllQueuesEmpty()
{
    if (!m_NeighbourSetRasterTiles.empty()) return false;
    if (!m_NeighbourSetRasterTiles.empty()) return false;
    if (!m_QueueRasterTiles.empty()) return false;
    if (!m_QueueTile3Ds.empty()) return false;

    return true;
}

void TileManager::AddRasterTileToQueue(RasterTileIndex index)
{
    RasterTileData tileData = m_tileManagerData.GetRasterTile(index.zoom, index.x, index.y);
    std::lock_guard<std::mutex> lock(m_MutexQueueRasterTiles);
    m_QueueRasterTiles[index] = tileData;
}

void TileManager::AddTile3DToQueue(Tile3DIndex index)
{
    Tile3DData tileData = m_tileManagerData.GetTile3D(index.lat, index.lon);
    std::lock_guard<std::mutex> lock(m_MutexQueueTile3Ds);
    m_QueueTile3Ds[index] = tileData;
}
void TileManager::GenerateRasterTileNeighbours()
{
    int k = m_config.NeighbourhoodFetchSizeRasterTile;

    int centerX;
    int centerY;

    int zoom = (int)GetZoom();

    centerX = long2tilex(m_lon, zoom);
    centerY = lat2tiley(m_lat, zoom);
    int nTiles = std::exp2(zoom);

    for (int i = -k; i <= k; i++)
    {
        // check if we are out of bounds
        if ((i + centerX) < 0 || (i + centerX) >= nTiles)
            continue;
        for (int j = -k; j <= k; j++)
        {
            if ((j + centerY) < 0 || (j + centerY) >= nTiles)
                continue;

            RasterTileIndex index = { zoom, i + centerX, j + centerY };
            std::lock_guard<std::mutex> lock(m_MutexNeighbourSetRasterTiles);
            m_NeighbourSetRasterTiles.insert(index);
        }
    }

}

void TileManager::GenerateTile3DNeighbours()
{
    int k = m_config.NeighbourhoodFetchSizeTile3D;
    double size = m_config.Tile3DSize;

    double centerLat = std::floor(m_lat / size) * size;
    double centerLon = std::floor(m_lon / size) * size;

    for (int i = -k; i <= k; i++)
    {
        // check if we are out of bounds
        if ((i * size + centerLat) < -89 || (i * size + centerLat) >= 89)
            continue;
        for (int j = -k; j <= k; j++)
        {
            if ((j * size + centerLon) < -180 || (j * size + centerLon) >= 180)
                continue;

            double _lat = i * size + centerLat;
            double _lon = j * size + centerLon;

            Tile3DIndex index = { _lat, _lon };
            std::lock_guard<std::mutex> lock(m_MutexNeighbourSetTile3Ds);
            m_NeighbourSetTile3Ds.insert(index);
        }
    }
}
// Generate requests for the Raster Tile for for every index in the neighbour set
void TileManager::GetRasterTileNeighbours()
{

    for (const RasterTileIndex& itr : m_NeighbourSetRasterTiles)
    {
        const RasterTileIndex& index = itr;

        std::thread t(&TileManager::AddRasterTileToQueue, this, index);
        t.detach();
    }

    // Once the request is made, remove the index from the set
    std::lock_guard<std::mutex> lock(m_MutexNeighbourSetRasterTiles);
    m_NeighbourSetRasterTiles.clear();
}

// Generate request for the Tile 3D for every index in the neighbour set 
void TileManager::GetTile3DNeighbours()
{
    for (const Tile3DIndex& itr : m_NeighbourSetTile3Ds)
    {
        const Tile3DIndex& index = itr;

        std::thread t(&TileManager::AddTile3DToQueue, this, index);
        t.detach();
    }

    // Once the request is made, remove the index from the set
    std::lock_guard<std::mutex> lock(m_MutexNeighbourSetTile3Ds);
    m_NeighbourSetTile3Ds.clear();
}

void TileManager::RemoveRasterTiles()
{
    int gridSize = m_config.NeighbourhoodFetchSizeRasterTile * 2 + 1;

    // Create a vector that will store the indices that need to be removed
    std::vector<RasterTileIndex> indices;
    indices.resize(gridSize * gridSize);

    // Get the indices to be removed from the queue
    for (auto const& element : m_QueueRasterTiles)
    {
        if (m_NeighbourSetRasterTiles.find(element.first) == m_NeighbourSetRasterTiles.end())
        {
            indices.push_back(element.first);
        }
    }

    // Remove Raster Tiles from the queue
    std::lock_guard<std::mutex> lockQueue(m_MutexQueueRasterTiles);
    for (auto& index : indices)
    {
        m_QueueRasterTiles.erase(index);
    }

    // clear the indices
    indices.clear();

    // Get the indices to be removed from the active list
    //for (auto const& element : m_ActiveRasterTiles)
    for (auto const& element : m_BackgroundRasterTiles)
    {
        if (m_NeighbourSetRasterTiles.find(element.first) == m_NeighbourSetRasterTiles.end())
        {
            indices.push_back(element.first);
        }
    }

    // Remove Raster Tiles from the active list
    std::lock_guard<std::mutex> lockActive(m_MutexActiveRasterTiles);
    for (auto& index : indices)
    {
        //m_ActiveRasterTiles.erase(index);
        m_BackgroundRasterTiles.erase(index);
    }
}

void TileManager::RemoveTile3Ds()
{
    int gridSize = m_config.NeighbourhoodFetchSizeTile3D * 2 + 1;

    // Create a vector that will store the indices that need to be removed
    std::vector<Tile3DIndex> indices;
    indices.resize(gridSize * gridSize);

    // Get the indices to be removed from the queue
    for (auto const& element : m_QueueTile3Ds)
    {
        if (m_NeighbourSetTile3Ds.find(element.first) == m_NeighbourSetTile3Ds.end())
        {
            indices.push_back(element.first);
        }
    }

    // Remove Raster Tiles from the queue
    std::lock_guard<std::mutex> lockQueue(m_MutexQueueTile3Ds);
    for (auto& index : indices)
    {
        m_QueueTile3Ds.erase(index);
    }

    // clear the indices
    indices.clear();

    // Get the indices to be removed from the active list
    //for (auto const& element : m_ActiveTile3Ds)
    for (auto const& element : m_BackgroundTile3Ds)
    {
        if (m_NeighbourSetTile3Ds.find(element.first) == m_NeighbourSetTile3Ds.end())
        {
            indices.push_back(element.first);
        }
    }

    // Remove Raster Tiles from the active list
    std::lock_guard<std::mutex> lockActive(m_MutexActiveTile3Ds);
    for (auto& index : indices)
    {
        //m_ActiveTile3Ds.erase(index);
        m_BackgroundTile3Ds.erase(index);
    }

}

void TileManager::PruneNeighbourSetRasterTile()
{
    std::vector<RasterTileIndex> indices;

    for (const auto& element : m_NeighbourSetRasterTiles)
    {
        if (m_QueueRasterTiles.find(element) != m_QueueRasterTiles.end())
        {
            indices.push_back(element);
        }
        else if (m_BackgroundRasterTiles.find(element) != m_BackgroundRasterTiles.end())
        {
            indices.push_back(element);
        }
    }

    // Remove already present tiles from the neighbour set
    std::lock_guard<std::mutex> lockActive(m_MutexNeighbourSetRasterTiles);
    for (auto& index : indices)
    {
        m_NeighbourSetRasterTiles.erase(index);
    }
}

void TileManager::PruneNeighbourSetTile3D()
{
    std::vector<Tile3DIndex> indices;

    for (const auto& element : m_NeighbourSetTile3Ds)
    {
        if (m_QueueTile3Ds.find(element) != m_QueueTile3Ds.end())
        {
            indices.push_back(element);
        }
        else if (m_BackgroundTile3Ds.find(element) != m_BackgroundTile3Ds.end())
        {
            indices.push_back(element);
        }
    }

    // Remove already present tiles from the neighbour set
    std::lock_guard<std::mutex> lockActive(m_MutexNeighbourSetTile3Ds);
    for (auto& index : indices)
    {
        //m_ActiveTile3Ds.erase(index);
        m_NeighbourSetTile3Ds.erase(index);
    }
}

void TileManager::SetPosition(double lat, double lon, double altitude)
{
    double EPSILON = 1e-6;
    
    if ((fabs(m_lat - lat) < EPSILON) && (fabs(m_lon - lon) < EPSILON) && (GetZoom() == GetZoomAtAltitude(altitude)))
    {
        return;
    }

    m_lat = lat;
    m_lon = lon;
    m_altitude = altitude;

    m_moved = true;
}

void TileManager::Update()
{

    AddRasterTileFromQueue();
    AddTile3DFromQueue();

    // Minimum refresh rate 
    // Why -> sometimes the API takes an unusually long amount of time to respond. 
    // In those cases no new tiles will not be shown until the very last tile arrives
    // Setting a minimum refresh rate would force the update even if a few tiles are missing. 
    // The missing tiles will be rendered once they eventually arrive
    int maxTimeToNextUpdate = 1000000; // in micro-seconds
    
    auto now = std::chrono::high_resolution_clock::now();

    auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_lastUpdateTimePoint).time_since_epoch().count();
    auto end = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch().count();

    // Calculate the time since last update
    auto duration = end - start;


    if (!m_moved && (duration < maxTimeToNextUpdate))
        return;
    
    if ((duration > maxTimeToNextUpdate) || (AllBackgroundTilesProcessed() && AllQueuesEmpty()))
    {
        m_lastUpdateFinished = true;
        m_lastUpdateTimePoint = std::chrono::high_resolution_clock::now();
        std::swap(m_ActiveRasterTiles, m_BackgroundRasterTiles);
        std::swap(m_ActiveTile3Ds, m_BackgroundTile3Ds);
    }
    
    if (!m_lastUpdateFinished)
        return;
    


    m_moved = false;
    m_lastUpdateFinished = false;
    
    GenerateRasterTileNeighbours();
    GenerateTile3DNeighbours();
    
    RemoveRasterTiles();
    RemoveTile3Ds();

    PruneNeighbourSetRasterTile();
    PruneNeighbourSetTile3D();

    GetRasterTileNeighbours(); 
    GetTile3DNeighbours();
    
}

void TileManager::AddRasterTileFromQueue()
{
    std::lock_guard<std::mutex> lockQueue(m_MutexQueueRasterTiles);
    if (m_QueueRasterTiles.empty())
    {
        return;
    }

    //Timer t("AddRasterTileFromQueue");
    auto const& element = *(m_QueueRasterTiles.begin());
    RasterTileRender tile(element.second, m_config.ReferencePoint.lat, m_config.ReferencePoint.lon);
    //t.~Timer();

    std::lock_guard<std::mutex> lockActive(m_MutexActiveRasterTiles);
    //m_ActiveRasterTiles[element.first] = tile;
    m_BackgroundRasterTiles[element.first] = tile;

    m_QueueRasterTiles.erase(element.first);
}

void TileManager::AddTile3DFromQueue()
{
    std::lock_guard<std::mutex> lockQueue(m_MutexQueueTile3Ds);
    if (m_QueueTile3Ds.empty())
    {
        return;
    }

    //Timer t("AddTile3DFromQueue");
    auto const& element = *(m_QueueTile3Ds.begin());
    Tile3DRender tile(element.second, m_config.ReferencePoint.lat, m_config.ReferencePoint.lon);
    //t.~Timer();
    
    std::lock_guard<std::mutex> lockActive(m_MutexActiveTile3Ds);
    //m_ActiveTile3Ds[element.first] = tile;
    m_BackgroundTile3Ds[element.first] = tile;

    m_QueueTile3Ds.erase(element.first);
}

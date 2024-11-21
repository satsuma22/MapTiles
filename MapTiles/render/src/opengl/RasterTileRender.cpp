#include "opengl/RasterTileRender.h"
#include "utils.h"
#include "WGS84toCartesian.hpp"

#include <iostream>


RasterTileRender::RasterTileRender(const RasterTileData& tile, double refLat, double refLon)
	: m_Texture(nullptr), m_VertexArray(nullptr), m_VertexBuffer(nullptr)
{

    m_Texture = std::make_shared<Texture>(tile.m_image, tile.m_height, tile.m_width);
    m_VertexArray = std::make_shared<VertexArray>();

    // Calculate the bounding quad
    double top = tiley2lat(tile.m_y, tile.m_zoom);
    double bottom = tiley2lat(tile.m_y + 1, tile.m_zoom);
    double left = tilex2long(tile.m_x, tile.m_zoom);
    double right = tilex2long(tile.m_x + 1, tile.m_zoom);


    std::array<double, 2> bottomLeftCartesian = wgs84::toCartesian({ refLat, refLon }, { bottom, left });
    std::array<double, 2> topRightCartesian = wgs84::toCartesian({ refLat, refLon }, { top, right });

    //std::cout << "Tile: (" << bottomLeftCartesian[0] << ", " << bottomLeftCartesian[1] << ")\n";

    
    float box[] =
    {
        (float)bottomLeftCartesian[0], 0, -(float)bottomLeftCartesian[1], 0, 0,
        (float)topRightCartesian[0]  , 0, -(float)bottomLeftCartesian[1], 1, 0,
        (float)topRightCartesian[0]  , 0, -(float)topRightCartesian[1]  , 1, 1,
        (float)bottomLeftCartesian[0], 0, -(float)bottomLeftCartesian[1], 0, 0,
        (float)topRightCartesian[0]  , 0, -(float)topRightCartesian[1]  , 1, 1,
        (float)bottomLeftCartesian[0], 0, -(float)topRightCartesian[1]  , 0, 1,
    };

    m_VertexBuffer = std::make_shared<VertexBuffer>(box, sizeof(box));

    VertexBufferLayout layout;
    layout.Push<float>(3);
    layout.Push<float>(2);

    m_VertexArray->AddBuffer(*m_VertexBuffer, layout);
}

RasterTileRender::~RasterTileRender()
{
 
}

#pragma once

struct GlobalConfig
{
	struct ReferencePointWGS84
	{
		double lat;
		double lon;
	};

	ReferencePointWGS84 ReferencePoint = { 0, 0 };
	double Tile3DSize = 0.01;	// in degrees

	// Detemines how many neighbours will be fetched
	// For a neighbourhood size of n, a (n * 2 + 1) x (n * 2 + 1) grid will be fetched
	int NeighbourhoodFetchSizeRasterTile = 2;
	int NeighbourhoodFetchSizeTile3D = 1;

	bool FrustumBasedTileGeneration = false;
	int FrustumRasterTilesCount = 10;
	double FrustumTile3DMaxDistance = 0.03;

	int MaxFPS = -1;

	int MaxRasterTileRequestThreads = 10;
	int MaxTile3DRequestThreads = 10;
};
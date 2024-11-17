#pragma once

// See: https://wiki.openstreetmap.org/wiki/Slippy_map_tilenames
int long2tilex(double lon, int z);

int lat2tiley(double lat, int z);

double tilex2long(int x, int z);

double tiley2lat(int y, int z);


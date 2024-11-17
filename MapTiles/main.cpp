#include <iostream>

#include "Application.h"
#include "Config.h"

#include <map>
#include <string>

int main(int argc, char** argv)
{
	struct Point
	{
		double lat;
		double lon;
	};

	std::map<std::string, Point> coordinates;
	coordinates["Dresden"]	= { 51.02596, 13.7230 };
	coordinates["Berlin"]	= { 52.5040, 13.3931 };
	coordinates["Munich"]	= { 48.1442, 11.5528 };
	coordinates["NewYork"]	= { 40.7585, -73.9861 };

	GlobalConfig config;
	
	if (argc == 1)
	{
		// Dresden
		config.ReferencePoint = { 51.02596, 13.7230 };
	}

	if (argc == 2)
	{
		if (coordinates.find(argv[1]) != coordinates.end())
		{
			Point p = coordinates[argv[1]];
			config.ReferencePoint = { p.lat, p.lon };
		}
		else
		{
			std::cout << "Coordinates for " << argv[1] << " not found. Please enter them manually.\n";
			std::cout << "Usage: MapTiles.exe <CityName>\t OR \t MapTiles.exe <latitude> <longitude>\n";
			return 0;
		}
	}

	if (argc == 3)
	{
		double lat;
		double lon;
		try 
		{
			lat = std::stod(argv[1]);
			lon = std::stod(argv[2]);
		}
		catch (...)
		{
			std::cout << "Usage: MapTiles.exe <CityName>\t OR \t MapTiles.exe <latitude> <longitude>\n";
			return 0;
		}

		config.ReferencePoint = { lat , lon };
	}
	if (argc > 3)
	{
		std::cout << "Usage: MapTiles.exe <CityName>\t OR \t MapTiles.exe <latitude> <longitude>\n";
		return 0;
	}
	
	Application app(config.ReferencePoint.lat, config.ReferencePoint.lon, config);

	app.Init();
	app.Run();
	app.Terminate();

	return 0;
}
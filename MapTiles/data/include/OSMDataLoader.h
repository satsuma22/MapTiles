#pragma once

#include <httplib.h>
#include <string>

/// <summary>
/// Class to construct an overpass API query and fetch the response
/// </summary>
class OSMDataLoader
{
public:
	OSMDataLoader(double latMin, double lonMin, double latMax, double lonMax);
	void FetchOSMWays();
	std::string& GetResponse() { return m_result->body; }
	int GetErrorStatus() { return (int)m_result.error(); }
	int GetHTTPStatus() { return m_result->status; }

private:
	std::string m_url;
	std::string m_query;
	double m_latMin, m_lonMin, m_latMax, m_lonMax;
	httplib::Result m_result;
};

#include <lanelet2_core/primitives/Lanelet.h>
#include <lanelet2_core/geometry/LineString.h>
#include <lanelet2_io/Io.h>
#include "pugixml.hpp"
#include <iostream>
#include <lanelet2_projection/UTM.h>

int main()
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file("data/mapping_example.osm");
    if (!result)
        return -1;
        
    pugi::xml_node firstNode = doc.child("osm").child("node");
    double latitude = firstNode.attribute("lat").as_double();
    double longitude = firstNode.attribute("lon").as_double();
    std::cout << "Latitude: " << latitude << ", Longitude: " << longitude << std::endl;
    lanelet::projection::UtmProjector projector(lanelet::Origin({latitude, longitude}));
    lanelet::LaneletMapPtr map = lanelet::load("data/mapping_example.osm", projector);
    std::cout << "Loaded map with " << map->laneletLayer.size() << " lanelets."  << map->pointLayer.size() << " points." << std::endl;
    std::cout << map->lineStringLayer.size() << " line strings." << map->regulatoryElementLayer.size() << " regulatory elements." << std::endl;
    std::cout << map->areaLayer.size() << " areas." <<  map->polygonLayer.size() << " polygons." << std::endl;

    double totalLength = 0.0;
    for (const auto& ll : map->laneletLayer) {
        totalLength += lanelet::geometry::length(ll.centerline2d());
    }
    std::cout << "Total lane length: " << totalLength / 1000.0 << " km" << std::endl;
}
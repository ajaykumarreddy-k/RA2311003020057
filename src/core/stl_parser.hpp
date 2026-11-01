#pragma once

#include <vector>
#include <string>
#include <istream>

namespace workbench {

struct StlVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct StlFacet {
    StlVertex normal;
    StlVertex v1;
    StlVertex v2;
    StlVertex v3;
};

struct StlModel {
    std::string solid_name;
    std::vector<StlFacet> facets;
    StlVertex bbox_min;
    StlVertex bbox_max;

    size_t triangle_count() const {
        return facets.size();
    }

    size_t vertex_count() const {
        return facets.size() * 3;
    }

    // Flat arrays for WebGL / Three.js BufferGeometry
    std::vector<float> flat_positions() const;
    std::vector<float> flat_normals() const;
};

// Parse ASCII STL from an input stream with strict validation
StlModel parse_ascii_stl_stream(std::istream& in);

// Parse ASCII STL from file path
StlModel parse_ascii_stl_file(const std::string& filepath);

// Parse ASCII STL from string content
StlModel parse_ascii_stl_string(const std::string& content);

} // namespace workbench

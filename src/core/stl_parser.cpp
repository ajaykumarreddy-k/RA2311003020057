#include "stl_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cctype>

namespace workbench {

static std::string to_lower_trimmed(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    if (start == str.size()) return "";
    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
        end--;
    }
    std::string res;
    res.reserve(end - start + 1);
    for (size_t i = start; i <= end; ++i) {
        res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(str[i]))));
    }
    return res;
}

static bool parse_finite_float(std::istream& is, float& val) {
    if (!(is >> val)) {
        return false;
    }
    return std::isfinite(val);
}

std::vector<float> StlModel::flat_positions() const {
    std::vector<float> pos;
    pos.reserve(facets.size() * 9);
    for (const auto& f : facets) {
        pos.push_back(f.v1.x);
        pos.push_back(f.v1.y);
        pos.push_back(f.v1.z);
        pos.push_back(f.v2.x);
        pos.push_back(f.v2.y);
        pos.push_back(f.v2.z);
        pos.push_back(f.v3.x);
        pos.push_back(f.v3.y);
        pos.push_back(f.v3.z);
    }
    return pos;
}

std::vector<float> StlModel::flat_normals() const {
    std::vector<float> norm;
    norm.reserve(facets.size() * 9);
    for (const auto& f : facets) {
        // Normal repeated for all 3 vertices of the facet
        for (int v = 0; v < 3; ++v) {
            norm.push_back(f.normal.x);
            norm.push_back(f.normal.y);
            norm.push_back(f.normal.z);
        }
    }
    return norm;
}

StlModel parse_ascii_stl_stream(std::istream& in) {
    StlModel model;
    std::string line;
    int line_num = 0;

    // Check if stream has any content
    if (!std::getline(in, line)) {
        throw std::invalid_argument("Empty STL file: file contains 0 bytes or cannot be read.");
    }
    line_num++;

    std::string trimmed = to_lower_trimmed(line);
    while (trimmed.empty() && std::getline(in, line)) {
        line_num++;
        trimmed = to_lower_trimmed(line);
    }

    if (trimmed.empty()) {
        throw std::invalid_argument("Empty STL file: file contains only whitespace.");
    }

    // Expect "solid" keyword
    if (trimmed.rfind("solid", 0) != 0) {
        throw std::invalid_argument("Invalid STL header at line " + std::to_string(line_num) +
                                    ": expected keyword 'solid', found '" + line + "'.");
    }

    if (line.size() > 5) {
        model.solid_name = line.substr(5);
        // Trim leading space
        while (!model.solid_name.empty() && std::isspace(static_cast<unsigned char>(model.solid_name.front()))) {
            model.solid_name.erase(0, 1);
        }
    }

    bool in_facet = false;
    bool in_loop = false;
    int vertex_count = 0;
    StlFacet current_facet;

    while (std::getline(in, line)) {
        line_num++;
        std::string lower = to_lower_trimmed(line);
        if (lower.empty()) continue;

        std::istringstream iss(lower);
        std::string token;
        iss >> token;

        if (token == "facet") {
            if (in_facet) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": unexpected 'facet' inside an unclosed facet.");
            }
            std::string normal_kw;
            iss >> normal_kw;
            if (normal_kw != "normal") {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": expected 'facet normal', found 'facet " + normal_kw + "'.");
            }
            if (!parse_finite_float(iss, current_facet.normal.x) ||
                !parse_finite_float(iss, current_facet.normal.y) ||
                !parse_finite_float(iss, current_facet.normal.z)) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": facet normal components must be finite real numbers.");
            }
            in_facet = true;
            in_loop = false;
            vertex_count = 0;
        } else if (token == "outer") {
            if (!in_facet || in_loop) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": unexpected 'outer loop'.");
            }
            std::string loop_kw;
            iss >> loop_kw;
            if (loop_kw != "loop") {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": expected 'outer loop', found 'outer " + loop_kw + "'.");
            }
            in_loop = true;
        } else if (token == "vertex") {
            if (!in_facet || !in_loop) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": 'vertex' keyword found outside 'outer loop'.");
            }
            StlVertex v;
            if (!parse_finite_float(iss, v.x) ||
                !parse_finite_float(iss, v.y) ||
                !parse_finite_float(iss, v.z)) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": vertex coordinates must be finite real numbers.");
            }
            if (vertex_count == 0) current_facet.v1 = v;
            else if (vertex_count == 1) current_facet.v2 = v;
            else if (vertex_count == 2) current_facet.v3 = v;
            else {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": facet has more than 3 vertices (only triangular facets supported).");
            }
            vertex_count++;
        } else if (token == "endloop") {
            if (!in_loop || vertex_count != 3) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": 'endloop' reached with " + std::to_string(vertex_count) + " vertices (expected 3).");
            }
            in_loop = false;
        } else if (token == "endfacet") {
            if (!in_facet || in_loop) {
                throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                            ": 'endfacet' reached while loop still open or facet missing.");
            }
            model.facets.push_back(current_facet);
            in_facet = false;
        } else if (token == "endsolid") {
            break;
        } else {
            throw std::invalid_argument("Malformed STL at line " + std::to_string(line_num) +
                                        ": unexpected token '" + token + "'.");
        }
    }

    if (in_facet || in_loop) {
        throw std::invalid_argument("Incomplete STL file: reached EOF with unclosed facet or loop.");
    }

    if (model.facets.empty()) {
        throw std::invalid_argument("Empty STL model: file contains solid header but 0 valid facets.");
    }

    // Compute bounding box
    model.bbox_min = model.facets[0].v1;
    model.bbox_max = model.facets[0].v1;

    auto update_bbox = [&](const StlVertex& v) {
        model.bbox_min.x = std::min(model.bbox_min.x, v.x);
        model.bbox_min.y = std::min(model.bbox_min.y, v.y);
        model.bbox_min.z = std::min(model.bbox_min.z, v.z);
        model.bbox_max.x = std::max(model.bbox_max.x, v.x);
        model.bbox_max.y = std::max(model.bbox_max.y, v.y);
        model.bbox_max.z = std::max(model.bbox_max.z, v.z);
    };

    for (const auto& f : model.facets) {
        update_bbox(f.v1);
        update_bbox(f.v2);
        update_bbox(f.v3);
    }

    return model;
}

StlModel parse_ascii_stl_string(const std::string& content) {
    std::istringstream iss(content);
    return parse_ascii_stl_stream(iss);
}

StlModel parse_ascii_stl_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::invalid_argument("Cannot open STL file: '" + filepath + "'. Check file path and read permissions.");
    }
    return parse_ascii_stl_stream(file);
}

} // namespace workbench

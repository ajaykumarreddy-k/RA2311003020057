#include "ansys_parser.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cctype>

namespace workbench {

static std::string trim_str(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        start++;
    }
    if (start == str.size()) return "";
    size_t end = str.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(str[end]))) {
        end--;
    }
    return str.substr(start, end - start + 1);
}

static std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> tokens;
    std::string token;
    bool in_quotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '"') {
            in_quotes = !in_quotes;
        } else if (c == ',' && !in_quotes) {
            tokens.push_back(trim_str(token));
            token.clear();
        } else if (c != '\r') {
            token.push_back(c);
        }
    }
    tokens.push_back(trim_str(token));
    return tokens;
}

std::vector<float> AnsysMeshResult::flat_positions() const {
    std::vector<float> pos;
    pos.reserve(nodes.size() * 3);
    for (const auto& n : nodes) {
        pos.push_back(static_cast<float>(n.x));
        pos.push_back(static_cast<float>(n.y));
        pos.push_back(static_cast<float>(n.z));
    }
    return pos;
}

std::vector<float> AnsysMeshResult::flat_temperatures() const {
    std::vector<float> temps;
    temps.reserve(nodes.size());
    for (const auto& n : nodes) {
        temps.push_back(static_cast<float>(n.temperature));
    }
    return temps;
}

std::vector<int64_t> AnsysMeshResult::flat_node_ids() const {
    std::vector<int64_t> ids;
    ids.reserve(nodes.size());
    for (const auto& n : nodes) {
        ids.push_back(n.node_id);
    }
    return ids;
}

AnsysMeshResult parse_ansys_csv_stream(std::istream& in) {
    AnsysMeshResult result;
    std::string line;
    int line_num = 0;

    // Check if empty
    if (!std::getline(in, line)) {
        throw std::invalid_argument("Empty CSV file: file contains 0 bytes or cannot be read.");
    }
    line_num++;

    std::string trimmed = trim_str(line);
    while (trimmed.empty() && std::getline(in, line)) {
        line_num++;
        trimmed = trim_str(line);
    }

    if (trimmed.empty()) {
        throw std::invalid_argument("Empty CSV file: file contains only whitespace.");
    }

    // Parse header
    auto header_tokens = split_csv_line(trimmed);
    if (header_tokens.size() < 5) {
        throw std::invalid_argument("Invalid CSV header at line " + std::to_string(line_num) +
                                    ": expected 5 columns (node_id,x_m,y_m,z_m,temperature_C), found " +
                                    std::to_string(header_tokens.size()) + " columns.");
    }

    // Locate column indices
    int idx_id = -1, idx_x = -1, idx_y = -1, idx_z = -1, idx_t = -1;
    for (size_t i = 0; i < header_tokens.size(); ++i) {
        std::string col = header_tokens[i];
        std::transform(col.begin(), col.end(), col.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });

        if (col == "node_id" || col == "id" || col == "node") idx_id = static_cast<int>(i);
        else if (col == "x_m" || col == "x") idx_x = static_cast<int>(i);
        else if (col == "y_m" || col == "y") idx_y = static_cast<int>(i);
        else if (col == "z_m" || col == "z") idx_z = static_cast<int>(i);
        else if (col == "temperature_c" || col == "temperature" || col == "temp_c" || col == "temp") idx_t = static_cast<int>(i);
    }

    if (idx_id == -1 || idx_x == -1 || idx_y == -1 || idx_z == -1 || idx_t == -1) {
        throw std::invalid_argument("Invalid CSV header at line " + std::to_string(line_num) +
                                    ": missing required columns. Required: node_id,x_m,y_m,z_m,temperature_C.");
    }

    // Parse rows
    while (std::getline(in, line)) {
        line_num++;
        std::string row_trimmed = trim_str(line);
        if (row_trimmed.empty() || row_trimmed[0] == '#') continue;

        auto tokens = split_csv_line(row_trimmed);
        if (tokens.size() <= static_cast<size_t>(std::max({idx_id, idx_x, idx_y, idx_z, idx_t}))) {
            throw std::invalid_argument("Malformed CSV row at line " + std::to_string(line_num) +
                                        ": insufficient column count (" + std::to_string(tokens.size()) + ").");
        }

        AnsysNode node;

        // Parse node_id
        try {
            size_t pos = 0;
            node.node_id = std::stoll(tokens[idx_id], &pos);
            if (pos != tokens[idx_id].size()) {
                throw std::invalid_argument("non-integer id");
            }
        } catch (...) {
            throw std::invalid_argument("Malformed CSV at line " + std::to_string(line_num) +
                                        ": invalid integer node_id '" + tokens[idx_id] + "'.");
        }

        // Helper to parse double and check finite
        auto parse_coord = [&](const std::string& str, const std::string& name) -> double {
            try {
                size_t pos = 0;
                double val = std::stod(str, &pos);
                if (pos != str.size() || !std::isfinite(val)) {
                    throw std::invalid_argument("non-finite");
                }
                return val;
            } catch (...) {
                throw std::invalid_argument("Malformed CSV at line " + std::to_string(line_num) +
                                            ": invalid finite floating-point value for " + name + " ('" + str + "').");
            }
        };

        node.x = parse_coord(tokens[idx_x], "x_m");
        node.y = parse_coord(tokens[idx_y], "y_m");
        node.z = parse_coord(tokens[idx_z], "z_m");
        node.temperature = parse_coord(tokens[idx_t], "temperature_C");

        result.nodes.push_back(node);
    }

    if (result.nodes.empty()) {
        throw std::invalid_argument("Empty CSV dataset: header found at line 1, but file contains 0 valid data rows.");
    }

    // Initialize min/max and bounding box
    result.min_temperature = result.nodes[0].temperature;
    result.max_temperature = result.nodes[0].temperature;
    result.bbox_min[0] = result.nodes[0].x;
    result.bbox_min[1] = result.nodes[0].y;
    result.bbox_min[2] = result.nodes[0].z;
    result.bbox_max[0] = result.nodes[0].x;
    result.bbox_max[1] = result.nodes[0].y;
    result.bbox_max[2] = result.nodes[0].z;

    for (const auto& n : result.nodes) {
        result.min_temperature = std::min(result.min_temperature, n.temperature);
        result.max_temperature = std::max(result.max_temperature, n.temperature);

        result.bbox_min[0] = std::min(result.bbox_min[0], n.x);
        result.bbox_min[1] = std::min(result.bbox_min[1], n.y);
        result.bbox_min[2] = std::min(result.bbox_min[2], n.z);

        result.bbox_max[0] = std::max(result.bbox_max[0], n.x);
        result.bbox_max[1] = std::max(result.bbox_max[1], n.y);
        result.bbox_max[2] = std::max(result.bbox_max[2], n.z);
    }

    return result;
}

AnsysMeshResult parse_ansys_csv_string(const std::string& content) {
    std::istringstream iss(content);
    return parse_ansys_csv_stream(iss);
}

AnsysMeshResult parse_ansys_csv_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::invalid_argument("Cannot open CSV file: '" + filepath + "'. Check file path and read permissions.");
    }
    return parse_ansys_csv_stream(file);
}

} // namespace workbench

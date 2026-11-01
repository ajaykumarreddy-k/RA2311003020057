#pragma once

#include <vector>
#include <string>
#include <istream>
#include <cstdint>

namespace workbench {

struct AnsysNode {
    int64_t node_id = 0;
    double x = 0.0;           // x in meters (m)
    double y = 0.0;           // y in meters (m)
    double z = 0.0;           // z in meters (m)
    double temperature = 0.0; // temperature in deg C
};

struct AnsysMeshResult {
    std::vector<AnsysNode> nodes;
    double min_temperature = 0.0;
    double max_temperature = 0.0;
    double bbox_min[3] = {0.0, 0.0, 0.0};
    double bbox_max[3] = {0.0, 0.0, 0.0};

    size_t node_count() const {
        return nodes.size();
    }

    std::vector<float> flat_positions() const;
    std::vector<float> flat_temperatures() const;
    std::vector<int64_t> flat_node_ids() const;
};

// Parse ANSYS thermal nodal CSV from input stream
AnsysMeshResult parse_ansys_csv_stream(std::istream& in);

// Parse ANSYS thermal nodal CSV from file
AnsysMeshResult parse_ansys_csv_file(const std::string& filepath);

// Parse ANSYS thermal nodal CSV from string
AnsysMeshResult parse_ansys_csv_string(const std::string& content);

} // namespace workbench

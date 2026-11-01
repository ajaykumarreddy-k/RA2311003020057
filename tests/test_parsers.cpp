#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "stl_parser.hpp"
#include "ansys_parser.hpp"

using namespace workbench;

TEST_CASE("Task 4 Parsers: ASCII STL Valid Synthetic Model") {
    auto model = parse_ascii_stl_file("data/synthetic_heat_sink.stl");

    CHECK(model.solid_name == "SYNTHETIC_HEAT_SINK_BRACKET");
    CHECK(model.triangle_count() == 48);
    CHECK(model.vertex_count() == 144);

    // Flat positions and normals
    auto positions = model.flat_positions();
    auto normals = model.flat_normals();
    CHECK(positions.size() == 48 * 9);
    CHECK(normals.size() == 48 * 9);

    // Bounding box: [0, 0.10] in x, [0, 0.05] in y, [0, 0.04] in z
    CHECK(model.bbox_min.x == doctest::Approx(0.0f));
    CHECK(model.bbox_max.x == doctest::Approx(0.10f));
    CHECK(model.bbox_min.y == doctest::Approx(0.0f));
    CHECK(model.bbox_max.y == doctest::Approx(0.05f));
    CHECK(model.bbox_min.z == doctest::Approx(0.0f));
    CHECK(model.bbox_max.z == doctest::Approx(0.04f));
}

TEST_CASE("Task 4 Parsers: ASCII STL Error Rejection and Explanations") {
    // 1. Empty string
    CHECK_THROWS_WITH_AS(
        parse_ascii_stl_string(""),
        doctest::Contains("Empty STL file"),
        std::invalid_argument
    );

    // 2. Whitespace only
    CHECK_THROWS_WITH_AS(
        parse_ascii_stl_string("   \n\t  \n"),
        doctest::Contains("Empty STL file"),
        std::invalid_argument
    );

    // 3. Header missing 'solid'
    CHECK_THROWS_WITH_AS(
        parse_ascii_stl_string("not_solid test\nendsolid\n"),
        doctest::Contains("expected keyword 'solid'"),
        std::invalid_argument
    );

    // 4. Solid with 0 facets
    CHECK_THROWS_WITH_AS(
        parse_ascii_stl_string("solid empty\nendsolid empty\n"),
        doctest::Contains("0 valid facets"),
        std::invalid_argument
    );

    // 5. Non-finite values (NaN / Inf)
    std::string nan_stl = 
        "solid test\n"
        "  facet normal 0.0 0.0 1.0\n"
        "    outer loop\n"
        "      vertex 0.0 NaN 0.0\n"
        "      vertex 1.0 0.0 0.0\n"
        "      vertex 0.0 1.0 0.0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test\n";
    CHECK_THROWS_WITH_AS(
        parse_ascii_stl_string(nan_stl),
        doctest::Contains("finite real numbers"),
        std::invalid_argument
    );

    // 6. Incomplete facet (only 2 vertices)
    std::string incomplete_stl = 
        "solid test\n"
        "  facet normal 0.0 0.0 1.0\n"
        "    outer loop\n"
        "      vertex 0.0 0.0 0.0\n"
        "      vertex 1.0 0.0 0.0\n"
        "    endloop\n"
        "  endfacet\n"
        "endsolid test\n";
    CHECK_THROWS_WITH_AS(
        parse_ascii_stl_string(incomplete_stl),
        doctest::Contains("expected 3"),
        std::invalid_argument
    );
}

TEST_CASE("Task 4 Parsers: ANSYS CSV Valid Synthetic Dataset") {
    auto mesh = parse_ansys_csv_file("data/synthetic_ansys_thermal.csv");

    CHECK(mesh.node_count() == 160);
    CHECK(mesh.nodes.front().node_id == 1);
    CHECK(mesh.nodes.back().node_id == 160);

    // Flat arrays
    auto positions = mesh.flat_positions();
    auto temps = mesh.flat_temperatures();
    auto ids = mesh.flat_node_ids();

    CHECK(positions.size() == 160 * 3);
    CHECK(temps.size() == 160);
    CHECK(ids.size() == 160);

    // Temperature range
    CHECK(mesh.min_temperature >= 19.9);
    CHECK(mesh.max_temperature <= 145.5);
    CHECK(mesh.max_temperature > mesh.min_temperature);

    // Bounding box
    CHECK(mesh.bbox_min[0] == doctest::Approx(0.0));
    CHECK(mesh.bbox_max[0] == doctest::Approx(0.10));
    CHECK(mesh.bbox_min[1] == doctest::Approx(0.0));
    CHECK(mesh.bbox_max[1] == doctest::Approx(0.05));
    CHECK(mesh.bbox_min[2] == doctest::Approx(0.0));
    CHECK(mesh.bbox_max[2] == doctest::Approx(0.02));
}

TEST_CASE("Task 4 Parsers: ANSYS CSV Error Rejection and Explanations") {
    // 1. Empty string
    CHECK_THROWS_WITH_AS(
        parse_ansys_csv_string(""),
        doctest::Contains("Empty CSV file"),
        std::invalid_argument
    );

    // 2. Missing columns
    CHECK_THROWS_WITH_AS(
        parse_ansys_csv_string("node_id,x_m,y_m\n1,0.0,0.0\n"),
        doctest::Contains("expected 5 columns"),
        std::invalid_argument
    );

    // 3. Valid header but zero data rows
    CHECK_THROWS_WITH_AS(
        parse_ansys_csv_string("node_id,x_m,y_m,z_m,temperature_C\n"),
        doctest::Contains("0 valid data rows"),
        std::invalid_argument
    );

    // 4. Non-finite values
    std::string nan_csv = 
        "node_id,x_m,y_m,z_m,temperature_C\n"
        "1,0.0,NaN,0.0,25.0\n";
    CHECK_THROWS_WITH_AS(
        parse_ansys_csv_string(nan_csv),
        doctest::Contains("invalid finite floating-point value"),
        std::invalid_argument
    );

    // 5. Malformed row (alphabetic node_id)
    std::string bad_id_csv = 
        "node_id,x_m,y_m,z_m,temperature_C\n"
        "abc,0.0,0.0,0.0,25.0\n";
    CHECK_THROWS_WITH_AS(
        parse_ansys_csv_string(bad_id_csv),
        doctest::Contains("invalid integer node_id"),
        std::invalid_argument
    );
}

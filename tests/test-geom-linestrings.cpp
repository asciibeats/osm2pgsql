/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This file is part of osm2pgsql (https://osm2pgsql.org/).
 *
 * Copyright (C) 2006-2022 by the osm2pgsql developer community.
 * For a full list of authors see the git log.
 */

#include <catch.hpp>

#include "common-buffer.hpp"

#include "geom-from-osm.hpp"
#include "geom-functions.hpp"
#include "geom.hpp"

#include <array>

TEST_CASE("geom::linestring_t", "[NoDB]")
{
    geom::linestring_t ls1;

    REQUIRE(ls1.empty());
    ls1.emplace_back(17, 42);
    ls1.emplace_back(-3, 22);
    REQUIRE(ls1.size() == 2);

    auto it = ls1.cbegin();
    REQUIRE(it != ls1.cend());
    REQUIRE(it->x() == 17);
    ++it;
    REQUIRE(it != ls1.cend());
    REQUIRE(it->y() == 22);
    ++it;
    REQUIRE(it == ls1.cend());

    REQUIRE(ls1.num_geometries() == 1);
}

TEST_CASE("line geometry", "[NoDB]")
{
    geom::geometry_t const geom{geom::linestring_t{{1, 1}, {2, 2}}};

    REQUIRE(num_geometries(geom) == 1);
    REQUIRE(area(geom) == Approx(0.0));
    REQUIRE(geometry_type(geom) == "LINESTRING");
    REQUIRE(centroid(geom) == geom::geometry_t{geom::point_t{1.5, 1.5}});
}

TEST_CASE("create_linestring from OSM data", "[NoDB]")
{
    test_buffer_t buffer;
    buffer.add_way("w20 Nn1x1y1,n2x2y2");

    auto const geom =
        geom::create_linestring(buffer.buffer().get<osmium::Way>(0));

    REQUIRE(geom.is_linestring());
    REQUIRE(geometry_type(geom) == "LINESTRING");
    REQUIRE(num_geometries(geom) == 1);
    REQUIRE(area(geom) == Approx(0.0));
    REQUIRE(geom.get<geom::linestring_t>() ==
            geom::linestring_t{{1, 1}, {2, 2}});
    REQUIRE(centroid(geom) == geom::geometry_t{geom::point_t{1.5, 1.5}});
}

TEST_CASE("create_linestring from OSM data without locations", "[NoDB]")
{
    test_buffer_t buffer;
    buffer.add_way("w20 Nn1,n2");

    auto const geom =
        geom::create_linestring(buffer.buffer().get<osmium::Way>(0));

    REQUIRE(geom.is_null());
}

TEST_CASE("create_linestring from invalid OSM data", "[NoDB]")
{
    test_buffer_t buffer;
    buffer.add_way("w20 Nn1x1y1");

    auto const geom =
        geom::create_linestring(buffer.buffer().get<osmium::Way>(0));

    REQUIRE(geom.is_null());
}

TEST_CASE("geom::segmentize w/o split", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 2}, {2, 2}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 10.0);

    REQUIRE(geom.is_multilinestring());
    REQUIRE(num_geometries(geom) == 1);
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 1);
    REQUIRE(ml[0] == line);
}

TEST_CASE("geom::segmentize with split 0.5", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}};

    std::array<geom::linestring_t, 2> const expected{
        geom::linestring_t{{0, 0}, {0.5, 0}},
        geom::linestring_t{{0.5, 0}, {1, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 0.5);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 2);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
}

TEST_CASE("geom::segmentize with split 0.4", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}};

    std::array<geom::linestring_t, 3> const expected{
        geom::linestring_t{{0, 0}, {0.4, 0}},
        geom::linestring_t{{0.4, 0}, {0.8, 0}},
        geom::linestring_t{{0.8, 0}, {1, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 0.4);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 3);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
}

TEST_CASE("geom::segmentize with split 1.0 at start", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {2, 0}, {3, 0}, {4, 0}};

    std::array<geom::linestring_t, 4> const expected{
        geom::linestring_t{{0, 0}, {1, 0}}, geom::linestring_t{{1, 0}, {2, 0}},
        geom::linestring_t{{2, 0}, {3, 0}}, geom::linestring_t{{3, 0}, {4, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 1.0);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 4);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
    REQUIRE(ml[3] == expected[3]);
}

TEST_CASE("geom::segmentize with split 1.0 in middle", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}, {3, 0}, {4, 0}};

    std::array<geom::linestring_t, 4> const expected{
        geom::linestring_t{{0, 0}, {1, 0}}, geom::linestring_t{{1, 0}, {2, 0}},
        geom::linestring_t{{2, 0}, {3, 0}}, geom::linestring_t{{3, 0}, {4, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 1.0);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 4);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
    REQUIRE(ml[3] == expected[3]);
}

TEST_CASE("geom::segmentize with split 1.0 at end", "[NoDB]")
{
    geom::linestring_t const line{{0, 0}, {1, 0}, {2, 0}, {4, 0}};

    std::array<geom::linestring_t, 4> const expected{
        geom::linestring_t{{0, 0}, {1, 0}}, geom::linestring_t{{1, 0}, {2, 0}},
        geom::linestring_t{{2, 0}, {3, 0}}, geom::linestring_t{{3, 0}, {4, 0}}};

    auto const geom = geom::segmentize(geom::geometry_t{line}, 1.0);

    REQUIRE(geom.is_multilinestring());
    auto const &ml = geom.get<geom::multilinestring_t>();
    REQUIRE(ml.num_geometries() == 4);
    REQUIRE(ml[0] == expected[0]);
    REQUIRE(ml[1] == expected[1]);
    REQUIRE(ml[2] == expected[2]);
    REQUIRE(ml[3] == expected[3]);
}
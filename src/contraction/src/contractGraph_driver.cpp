/*PGR-GNU*****************************************************************
File: contractGraph_driver.cpp

Generated with Template by:
Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

Function's developer: 
Copyright (c) 2016 Rohith Reddy
Mail: 

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/


#if defined(__MINGW32__) || defined(_MSC_VER)
#include <winsock2.h>
#include <windows.h>

#ifdef unlink
#undef unlink
#endif

#endif


// #define DEBUG
#include <string.h>
#include <sstream>
#include <deque>
#include <vector>
#include "./contractGraph_driver.h"

#include "./pgr_contract.hpp"

#include "./../../common/src/pgr_types.h"
#include "./structs.h"

#include "./../../common/src/pgr_alloc.hpp"
#include "./../../common/src/identifiers.hpp"


/************************************************************
    edges_sql TEXT,
    contraction_order BIGINT[],
    forbidden_vertices BIGINT[] DEFAULT ARRAY[]::BIGINT[],
    max_cycles integer DEFAULT 1,
    directed BOOLEAN DEFAULT true
 ***********************************************************/
    void
    do_pgr_contractGraph(
        pgr_edge_t  *data_edges,
        size_t total_edges,
        int64_t *forbidden_vertices,
        size_t size_forbidden_vertices,
        int64_t *contraction_order,
        size_t size_contraction_order,
        int64_t max_cycles,
        bool directed,
        pgr_contracted_blob **return_tuples,
        size_t *return_count,
        char ** err_msg) {
        std::ostringstream log;
        try {
            std::ostringstream debug;
            graphType gType = directed? DIRECTED: UNDIRECTED;
            std::vector< pgr_edge_t > edges(data_edges, data_edges + total_edges);
            std::vector < pgrouting::contraction::Vertex > vertices(pgrouting::contraction::extract_vertices(edges));
            Identifiers<int64_t> remaining_vertices;
            std::vector< pgrouting::contraction::Edge > shortcut_edges;

            log << "Original Graph: \n" <<
            std::setprecision(32);
            for (const auto edge : edges) {
                log << "id = " << edge.id
                << "\tsource = " << edge.source
                << "\ttarget = " << edge.target
                << "\tcost = " << edge.cost
                << "\treverse_cost = " << edge.reverse_cost
                << ")\n";
            }
            log << "size_contraction_order " << size_contraction_order << "\n";
            log << "contraction_order: " <<"{ ";
            for (size_t i = 0; i < size_contraction_order; ++i) {
                log << contraction_order[i] << ", ";
            }
            log << " }\n";

            log << "size_forbidden_vertices " << size_forbidden_vertices << "\n";
            log << "forbidden_vertices" << "{ ";
            for (size_t i = 0; i < size_forbidden_vertices; ++i) {
                log << forbidden_vertices[i] << ", ";
            }
            log << " }\n";
            log << "max_cycles " << max_cycles << "\n";
            log << "directed " << gType << "\n";

            if (directed) {
                log << "Working with directed Graph\n";
                pgrouting::CHDirectedGraph digraph(vertices, gType);
                digraph.graph_insert_data(data_edges, total_edges);
                log << "Checking for valid forbidden vertices\n";
                for (size_t i = 0; i < size_forbidden_vertices; ++i) {
                    if (!digraph.has_vertex(forbidden_vertices[i])) {
                        log << "Invalid forbidden vertex: " << forbidden_vertices[i] << "\n";
                        *err_msg = strdup(log.str().c_str());
                        return;
                    }
                }
                Identifiers<int64_t> forbid_vertices(forbidden_vertices,
                    size_forbidden_vertices);
                log << "Before contraction\n";
                digraph.print_graph(log);
                /* Function call to get the contracted graph. */
                pgr_contractGraph(digraph,
                    forbid_vertices,
                    contraction_order, size_contraction_order,
                    max_cycles, remaining_vertices,
                    shortcut_edges, debug);

                log << "After contraction\n";
                digraph.print_graph(log);
                log << debug.str().c_str() << "\n";
                (*return_tuples) = pgr_alloc(remaining_vertices.size()+shortcut_edges.size(), (*return_tuples));
                size_t sequence = 0;
                int i = 1;
                log << "Remaining Vertices:" << "\n";
                for (const auto vertex : remaining_vertices) {
                    log << vertex << "\n";
                }
                char *type;
                for (auto id : remaining_vertices) {
                    type = strdup("v");
                    int64_t *contracted_vertices = NULL;
                    int contracted_vertices_size = 0;
                    digraph.get_contracted_vertices(&contracted_vertices,
                        contracted_vertices_size, id);
                    (*return_tuples)[sequence] = {i, id, type, -1, -1, -1.00,
                        contracted_vertices, contracted_vertices_size};
                        i++;
                        ++sequence;
                    }
                    log << "Added Edges:" << "\n";
                    for (const auto edge : shortcut_edges) {
                        log << edge << "\n";
                    }
                    for (auto edge : shortcut_edges) {
                        type = strdup("e");
                        int64_t *contracted_vertices = NULL;
                        int contracted_vertices_size = 0;
                        digraph.get_ids(&contracted_vertices,
                            contracted_vertices_size, edge.contracted_vertices());
                        (*return_tuples)[sequence] = {i, edge.id, type,
                            edge.source, edge.target, edge.cost,
                            contracted_vertices, contracted_vertices_size};
                            i++;
                            ++sequence;
                        }

                        (*return_count) = sequence;
                        log << "Returning from driver\n";
                    } else {
                        log << "Working with Undirected Graph\n";

                        pgrouting::CHUndirectedGraph undigraph(vertices, gType);
                        undigraph.graph_insert_data(data_edges, total_edges);
                        log << "Checking for valid forbidden vertices\n";
                        for (size_t i = 0; i < size_forbidden_vertices; ++i) {
                            if (!undigraph.has_vertex(forbidden_vertices[i])) {
                                log << "Invalid forbidden vertex: " << forbidden_vertices[i] << "\n";
                                *err_msg = strdup(log.str().c_str());
                                return;
                            }
                        }
                        Identifiers<int64_t> forbid_vertices(forbidden_vertices,
                            size_forbidden_vertices);
                        log << "Before contraction\n";
                        undigraph.print_graph(log);
                        /* Function call to get the contracted graph. */
                        pgr_contractGraph(undigraph,
                            forbid_vertices,
                            contraction_order, size_contraction_order,
                            max_cycles, remaining_vertices,
                            shortcut_edges, debug);
                        log << debug.str().c_str() << "\n";
                        log << "After contraction\n";
                        undigraph.print_graph(log);
                        log << "Size of remaining_vertices: " << remaining_vertices.size() << std::endl;
                        (*return_tuples) = pgr_alloc(remaining_vertices.size()+shortcut_edges.size(), (*return_tuples));
                        size_t sequence = 0;
                        int i = 1;
                        log << "Remaining Vertices:" << "\n";
                        for (const auto vertex : remaining_vertices) {
                            log << vertex << "\n";
                        }
                        char *type;
                        for (auto id : remaining_vertices) {
                            type = strdup("v");
                            int64_t *contracted_vertices = NULL;
                            int contracted_vertices_size = 0;
                            undigraph.get_contracted_vertices(&contracted_vertices,
                                contracted_vertices_size, id);
                            (*return_tuples)[sequence] = {i, id, type, -1, -1, -1.00,
                                contracted_vertices, contracted_vertices_size};
                                i++;
                                ++sequence;
                            }
                            log << "Added Edges:" << "\n";
                            for (const auto edge : shortcut_edges) {
                                log << edge << "\n";
                            }
                            for (auto edge : shortcut_edges) {
                                type = strdup("e");
                                int64_t *contracted_vertices = NULL;
                                int contracted_vertices_size = 0;
                                undigraph.get_ids(&contracted_vertices,
                                    contracted_vertices_size, edge.contracted_vertices());
                                (*return_tuples)[sequence] = {i, edge.id, type,
                                    edge.source, edge.target, edge.cost,
                                    contracted_vertices, contracted_vertices_size};
                                    i++;
                                    ++sequence;
                                }
                                (*return_count) = sequence;
                            }
#ifndef DEBUG
                            *err_msg = strdup("OK");
#else
                            *err_msg = strdup(log.str().c_str());
#endif
                        }
                        catch (AssertFailedException &except) {
                            log << except.what() << "\n";
                            *err_msg = strdup(log.str().c_str());
                        } catch (std::exception& except) {
                            log << except.what() << "\n";
                            *err_msg = strdup(log.str().c_str());
                        } catch(...) {
                            log << "Caught unknown exception!\n";
                            *err_msg = strdup(log.str().c_str());
                        }
                    }

                    int is_valid_contraction(int64_t number) {
                        switch (number) {
                            case 1:
                            return 1;
                            break;
                            case 2:
                            return 1;
                            break;
                            default:
                            return -1;
                            break;
                        }
                    }







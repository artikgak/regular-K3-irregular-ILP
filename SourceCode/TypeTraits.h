#pragma once
#include <boost/pending/property.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>
using namespace boost;

////////////////////////////////////////////////////////////////////////////////////////////////////

using UndirectedGraph = adjacency_list<setS, vecS, undirectedS, no_property,
    property<edge_weight_t, int>>;

typedef boost::graph_traits<UndirectedGraph>::vertex_descriptor UndirectedGraphVertex;
typedef boost::graph_traits<UndirectedGraph>::vertex_iterator UndirectedGraphVertexIterator;

typedef boost::graph_traits<UndirectedGraph>::edge_descriptor UndirectedGraphEdge;
typedef boost::graph_traits<UndirectedGraph>::edge_iterator UndirectedGraphEdgeIterator;

////////////////////////////////////////////////////////////////////////////////////////////////////

using DiGraph = adjacency_list<setS, vecS, bidirectionalS, no_property,
    property<edge_weight_t, int>>;

typedef boost::graph_traits<DiGraph>::vertex_descriptor DiGraphVertex;
typedef boost::graph_traits<DiGraph>::vertex_iterator DiGraphVertexIterator;

typedef boost::graph_traits<DiGraph>::edge_descriptor DiGraphEdge;
typedef boost::graph_traits<DiGraph>::edge_iterator DiGraphEdgeIterator;

////////////////////////////////////////////////////////////////////////////////////////////////////

using MyDistanceMatrix = std::vector<std::vector<int> >;

typedef exterior_vertex_property<UndirectedGraph, int> DistanceProperty;
typedef DistanceProperty::matrix_type DistanceMatrix;
typedef DistanceProperty::matrix_map_type DistanceMatrixMap;

typedef graph_traits<UndirectedGraph>::edge_descriptor Edge;
// Declare the weight map so that each edge returns the same value.
typedef constant_property_map<Edge, int> WeightMap;

// Declare a container and its corresponding property map that
// will contain the resulting eccentricities of each vertex in
// the graph.
typedef boost::exterior_vertex_property<UndirectedGraph, int> EccentricityProperty;
typedef EccentricityProperty::container_type EccentricityContainer;
typedef EccentricityProperty::map_type EccentricityMap;

///////////////////////////////////////////////////////////////////////////////////////
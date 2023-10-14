#pragma once

#include <string>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// Short alias for this namespace
namespace pt = boost::property_tree;

namespace BehaviorGraph {

std::string toJson();
pt::ptree toPropertyTree();

};
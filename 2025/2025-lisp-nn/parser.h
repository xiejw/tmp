// vim: ft=cpp
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace luma {

typedef enum { NODE_LIST, NODE_SYMBOL, NODE_NUMBER } NodeType;

class Node {
  public:
    NodeType                           type;
    std::string                        sym;       // if SYMBOL
    double                             num;       // if NUMBER
    std::vector<std::unique_ptr<Node>> children;  // if LIST
};

std::unique_ptr<Node> parse_expr( std::string_view input );
void                  print( Node *n );

}  // namespace luma

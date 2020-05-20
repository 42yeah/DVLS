//
//  main.cpp
//  dvls
//
//  Created by 周昊 on 2020/5/20.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#include <iostream>
#include <vector>
#include <random>

#define MAXWIDTH 100


struct Node;

struct Path {
    std::vector<Node *> path;
    Node *destination;
    float cost;
};

struct Node {
    Node(int x, int y, float r, int id) : x(x), y(y), r(r), id(id) {
    }
    
    int x, y, id;
    float r;
    std::vector<std::pair<float, Node *>> neighbors;
    std::vector<Path> routes;
    
    Node *prev;
    float cost;
};

std::vector<Node> nodes;

std::pair<int, Node *> index(int x, int y) {
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].x == x && nodes[i].y == y) {
            return std::pair<int, Node *>(i, &nodes[i]);
        }
    }
    return std::pair<int, Node *>(-1, nullptr);
}

std::pair<int, Node *> index(int id) {
    for (int i = 0; i < nodes.size(); i++) {
        if (nodes[i].id == id) {
            return std::pair<int, Node *>(i, &nodes[i]);
        }
    }
    return std::pair<int, Node *>(-1, nullptr);
}

void discoverNeighbors() {
    for (int i = 0; i < nodes.size(); i++) {
        Node &a = nodes[i];
        for (int j = i + 1; j < nodes.size(); j++) {
            Node &b = nodes[j];
            float dx = a.x - b.x, dy = a.y - b.y;
            float dist = sqrtf(powf(dx, 2.0f) + powf(dy, 2.0f));
            if (dist < a.r) {
                // Radius is consistent
                std::pair<float, Node *> pair(dist, nullptr);
                pair.second = &b;
                a.neighbors.push_back(pair);
                pair.second = &a;
                b.neighbors.push_back(pair);
            }
        }
//        std::cout << a.id << " neighbors: ";
//        for (int i = 0; i < a.neighbors.size(); i++) {
//            std::cout << a.neighbors[i].second->id << ", ";
//        }
//        std::cout << std::endl;
    }
}

void discoverRoutes() {
    for (int i = 0; i < nodes.size(); i++) {
        Node &node = nodes[i];
        for (int j = 0; j < node.neighbors.size(); j++) {
            float cost = node.neighbors[j].first;
            Node *neighbor = node.neighbors[j].second;
            Path path;
            path.path.push_back(neighbor);
            path.destination = neighbor;
            path.cost = cost;
            node.routes.push_back(path);
        }
//        for (int i = 0; i < node.routes.size(); i++) {
//            std::cout << node.id << " -> ";
//            for (int j = 0; j < node.routes[i].path.size(); j++) {
//                std::cout << node.routes[i].path[j]->id << " -> ";
//            }
//            std::cout << "COST: " << node.routes[i].cost << std::endl;
//        }
    }
}

std::pair<bool, Path *> hasRouteTo(Node *src, Node *dst) {
    std::pair<bool, Path *> ret;
    ret.first = false;
    ret.second = nullptr;
    for (int i = 0; i < src->routes.size(); i++) {
        Path &path = src->routes[i];
        if (path.destination == dst) {
            ret.first = true;
            ret.second = &path;
            return ret;
        }
    }
    return ret;
}

const int threshold = 100;
int current = 0;

void printPath(Node node, Path path) {
    std::cout << node.id << " -> ";
    for (int j = 0; j < path.path.size(); j++) {
        std::cout << path.path[j]->id << " -> ";
    }
    std::cout << "花费: " << path.cost << std::endl;
}

Path appendPath(Node &append, float extraCost, Path orig) {
    Path path;
    path.cost = orig.cost + extraCost;
    path.path.push_back(&append);
    path.path.insert(path.path.end(), orig.path.begin(), orig.path.end());
    path.destination = orig.destination;
    return path;
}

std::pair<int, Path *> destinationIndex(Node &hay, Path needle) {
    std::pair<int, Path *> ret;
    ret.first = -2;
    if (needle.destination->id == hay.id) { ret.first = -1; return ret; }
    for (int i = 0; i < hay.routes.size(); i++) {
        if (hay.routes[i].destination->id == needle.destination->id) {
            ret.first = i;
            ret.second = &hay.routes[i];
            return ret;
        }
    }
    return ret;
}

// === DV CODES === //

int blah = 0;
const int blahThreshold = 100;

bool notifyNeighbors(Node &node) {
    std::vector<Path> &routes = node.routes;
    bool dirty = false;
    for (int i = 0; i < node.neighbors.size(); i++) {
        Node &notified = *node.neighbors[i].second;
        float cost = node.neighbors[i].first;
        
        for (int i = 0; i < routes.size(); i++) {
            std::pair<int, Path *> info = destinationIndex(notified, routes[i]);
            if (info.first == -2) {
                // Create new path
                Path newPath = appendPath(node, cost, routes[i]);
                notified.routes.push_back(newPath);
                dirty = true;
            } else if (info.first >= 0) {
                // Optimize path?
                Path newPath = appendPath(node, cost, routes[i]);
                Path &oldPath = *info.second;
                if (oldPath.cost > newPath.cost) {
                    blah++;
                    if (blah == blahThreshold) {
                        std::cout << "..." << std::endl;
                    } else if (blah < blahThreshold) {
                        std::cout << "路径优化: " << std::endl << "从 ";
                        printPath(notified, oldPath);
                        std::cout << "到 ";
                        printPath(notified, newPath);
                    }
                    oldPath.cost = newPath.cost;
                    oldPath.path = newPath.path;
                    dirty = true;
                }
            }
        }
    }
    return dirty;
}

void propagateDV() {
    bool dirty = true;
    while (dirty) {
        dirty = false;
        for (int i = 0; i < nodes.size(); i++) {
            // Notify itself to all neighboring nodes
            Node &node = nodes[i];
            dirty |= notifyNeighbors(node);
        }
    }
}

// === DV CODE END === //

// === LS CODE START === //


bool propagateLS(Node *src, Node *dst) {
    bool dirty = false;
    std::vector<Node *> frontier;
    frontier.push_back(src);
    
    while (!frontier.empty()) {
        Node *node = frontier[0];
        frontier.erase(frontier.begin(), frontier.begin() + 1);
        for (int i = 0; i < node->neighbors.size(); i++) {
            std::pair<float, Node *> neighbor = node->neighbors[i];
            
            // If there is no previous node,
            // Or if the previous cost is way higher than the new path
            if (!neighbor.second->prev ||
                neighbor.second->cost > neighbor.first + node->cost) {
                neighbor.second->prev = node;
                neighbor.second->cost = neighbor.first + node->cost;
                dirty = true;
                frontier.push_back(neighbor.second);
            }
        }
    }
    return dirty;
}

Path pathfindLS(Node *src, Node *dst) {
    Path path;
    path.destination = dst;
    path.cost = 0.0f;

    src->cost = 0.0f;
    for (int i = 0; i < nodes.size(); i++) {
        nodes[i].prev = nullptr;
    }
    bool dirty = true;
    while (dirty) {
        dirty = false;
        dirty |= propagateLS(src, dst);
    }
    
    Node *ptr = dst;
    if (!ptr->prev) {
        path.cost = -1.0f;
        return path;
    }
    while (ptr) {
        if (ptr == src) { break; }
        path.path.insert(path.path.begin(), ptr);
        ptr = ptr->prev;
        path.cost += ptr->cost;
    }
    return path;
}

// === LS CODE END === //


enum ImplementType {
    DV, LS
};

int main(int argc, const char * argv[]) {
    int numNodes;
    float nodeRadius;
    std::uniform_int_distribution<> distrib(0, MAXWIDTH);
    std::random_device dev;

    std::cout << "生成节点数量: ";
    std::cin >> numNodes;
    std::cout << "节点半径: ";
    std::cin >> nodeRadius;
    if (numNodes >= MAXWIDTH * MAXWIDTH) {
        std::cout << "节点数过多" << std::endl;
        return -2;
    }

    for (int i = 0; i < numNodes; i++) {
        bool valid = false;
        Node rand(0, 0, 0.0f, i + 1);
        while (!valid) {
            rand = Node(distrib(dev), distrib(dev), nodeRadius, i + 1);
            if (index(rand.x, rand.y).first < 0) {
                valid = true;
            }
        }
        nodes.push_back(rand);
    }
    discoverNeighbors();
    discoverRoutes();
    
    ImplementType type;
    std::cout << "实现方法？" << std::endl
              << "1. DV" << std::endl
              << "2. LS" << std::endl;
    int selection;
    std::cin >> selection;
    switch (selection) {
        case 1: type = DV; break;
        case 2: type = LS; break;
        default: std::cout << "未知类型" << std::endl; return 2;
    }

    // search for optimal paths in DV
    if (type == DV) { propagateDV(); }

//    for (int i = 0; i < nodes.size(); i++) {
//        Node &node = nodes[i];
//        for (int i = 0; i < node.routes.size(); i++) {
//            printPath(node, node.routes[i]);
//        }
//    }

    while (true) {
        int src, dst;
        std::cout << "出发节点: ";
        std::cin >> src;
        std::cout << "结束节点: ";
        std::cin >> dst;
        
        Node *srcNode = index(src).second, *dstNode = index(dst).second;
        if (!srcNode || !dstNode) {
            std::cout << "找不到节点" << std::endl;
            continue;
        }
        if (srcNode->id == dstNode->id) {
            std::cout << "起点 = 终点 -> 花费: 0" << std::endl;
            continue;
        }

        switch (type) {
            case DV: {
                std::pair<bool, Path *> info = hasRouteTo(srcNode, dstNode);
                if (info.first) {
                    std::cout << "路径: ";
                    printPath(*srcNode, *info.second);
                } else {
                    std::cout << "找不到路径" << std::endl;
                }
                break;
            }
                
            case LS: {
                Path path = pathfindLS(srcNode, dstNode);
                if (path.cost < 0.0f) {
                    std::cout << "找不到路径" << std::endl;
                } else {
                    std::cout << "路径: ";
                    printPath(*srcNode, path);
                }
                break;
            }
        }
        
    }
    
    return 0;
}

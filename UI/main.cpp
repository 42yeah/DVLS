//
//  main.cpp
//  UI
//
//  Created by Hao Zhou on 18/06/2020.
//  Copyright © 2020 aiofwa. All rights reserved.
//

#include <iostream>
#include <map>
#include <random>
#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <SFML/System.hpp>
#include <SFML/System/Vector2.hpp>
#define RETINA_MODIFIER 2
#define CAMSPEED 1000.0f
#define MAXWIDTH 100
#define WIDTH 10


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

bool intersects(sf::Vector2f pos, sf::Vector2f widget) {
    return !(pos.x < widget.x || pos.x > widget.x + WIDTH || pos.y < widget.y || pos.y > widget.y + WIDTH);
}

// === LS CODE END === //


enum ImplementType {
    DV, LS
};

int numNodes;
float nodeRadius;
ImplementType type;
int ptrMap[MAXWIDTH][MAXWIDTH] = { -1 };
Node *firstNode, *secondNode;

void generateNodes(int numNodes, float nodeRadius) {
    std::uniform_int_distribution<> distrib(0, MAXWIDTH - 1);
    std::random_device dev;
    std::memset(ptrMap, -1, sizeof(ptrMap));
    nodes.clear();
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
        ptrMap[rand.y][rand.x] = i;
    }
    discoverNeighbors();
    discoverRoutes();
    // search for optimal paths in DV
    if (type == DV) { propagateDV(); }
}

int main(int argc, const char * argv[]) {
    std::uniform_int_distribution<> distrib(0, MAXWIDTH);
    std::random_device dev;
    bool showAnnoyingTag = true;
    firstNode = secondNode = nullptr;

    sf::RenderWindow window(sf::VideoMode(800 * RETINA_MODIFIER, 600 * RETINA_MODIFIER), "DV/LS");
    sf::Color good(63, 150, 43);
    sf::Color busy(255, 125, 0);
    sf::Color idle(210, 210, 210);
    sf::Color warning(214, 32, 58);
    sf::Clock clock;
    clock.restart();
    std::memset(ptrMap, -1, sizeof(ptrMap));
    
    sf::Font noto;
    if (!noto.loadFromFile("Assets/noto.otf")) {
        return -1;
    }
    std::map<sf::Keyboard::Key, bool> keyMap;
    sf::Vector2u size = window.getSize();
    
    sf::Text title(sf::String(L"DV/LS 实验"), noto);
    title.setPosition(20.0f, 20.0f);
    title.setFillColor(sf::Color::White);
    
    tgui::Gui gui(window);
    tgui::Theme theme("themes/black.txt");
    auto numNodesElem = tgui::EditBox::create();
    numNodesElem->setRenderer(theme.getRenderer("EditBox"));
    numNodesElem->setSize(300, 50);
    numNodesElem->setTextSize(26);
    numNodesElem->setPosition(20, 70);
    numNodesElem->setInheritedFont(noto);
    numNodesElem->setDefaultText(L"生成数量...");
    gui.add(numNodesElem);
    
    auto radiusElem = tgui::EditBox::create();
    radiusElem->setRenderer(theme.getRenderer("EditBox"));
    radiusElem->setSize(300, 50);
    radiusElem->setTextSize(26);
    radiusElem->setPosition(20, 130);
    radiusElem->setInheritedFont(noto);
    radiusElem->setDefaultText(L"节点半径...");
    gui.add(radiusElem);
    
    auto dvElem = tgui::RadioButton::create();
    dvElem->setRenderer(theme.getRenderer("RadioButton"));
    dvElem->setPosition(20, 190);
    dvElem->setText("DV");
    dvElem->setSize(25, 25);
    dvElem->setInheritedFont(noto);
    dvElem->setTextSize(26);
    gui.add(dvElem);

    auto lsElem = tgui::RadioButton::create();
    lsElem->setRenderer(theme.getRenderer("RadioButton"));
    lsElem->setPosition(100, 190);
    lsElem->setText("LS");
    lsElem->setSize(25, 25);
    lsElem->setInheritedFont(noto);
    lsElem->setTextSize(26);
    gui.add(lsElem);
    
    auto button = tgui::Button::create();
    button->setRenderer(theme.getRenderer("Button"));
    button->setPosition(20, 240);
    button->setText(L"生成");
    button->setSize(200, 50);
    button->setTextSize(26);
    button->setInheritedFont(noto);
    button->onPress([=]{
        if (!dvElem->isChecked() && !lsElem->isChecked()) { return; }
        numNodes = numNodesElem->getText().toInt();
        nodeRadius = radiusElem->getText().toFloat();
        type = dvElem->isChecked() ? DV : LS;
        generateNodes(numNodes, nodeRadius);
    });
    gui.add(button);
    
    sf::View view;
    view.setCenter(0.0f, 0.0f);
    view.setSize(window.getSize().x, window.getSize().y);
    sf::Vector2f dest = { 0.0f, 0.0f };
    
    sf::Color regular(34, 34, 34);
    sf::Color node(231, 245, 13);
    sf::Color transparentNode = node;
    transparentNode.a = 25;
    sf::Color lessTransparentNode(250, 248, 100);
    lessTransparentNode.a = 175;
    float zoom = 1.0;
    float bottomBannerY = size.y + 10.0f;
    sf::Color bannerColor = warning;
    struct {
        int index;
        sf::Vector2f pos;
    } info;
    
    std::pair<int, Path> path = std::pair<bool, Path>(false, Path());
    while (window.isOpen()) {
        sf::Event e;
        size = window.getSize();
        float deltaTime = clock.getElapsedTime().asSeconds();
        clock.restart();
        
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed) { window.close(); }
            if (e.type == sf::Event::KeyPressed) {
                keyMap[e.key.code] = true;
                if (e.key.code == sf::Keyboard::Space) {
                    showAnnoyingTag = !showAnnoyingTag;
                }
            }
            if (e.type == sf::Event::KeyReleased) {
                keyMap[e.key.code] = false;
            }
            if (e.type == sf::Event::MouseWheelScrolled) {
                zoom += (e.mouseWheelScroll.delta * deltaTime) * 0.01f;
                view.zoom(zoom);
            }
            if (e.type == sf::Event::MouseButtonPressed) {
                if (info.index != -1) {
                    path.first = 0;
                    if (!firstNode) {
                        firstNode = &nodes[info.index];
                    } else {
                        secondNode = &nodes[info.index];
                    }
                } else {
                    firstNode = secondNode = nullptr;
                    path.first = 0;
                }
            }
            gui.handleEvent(e);
        }
        if (keyMap[sf::Keyboard::W]) {
            dest.y -= 1000.0f * zoom * deltaTime;
        }
        if (keyMap[sf::Keyboard::A]) {
            dest.x -= 1000.0f * zoom * deltaTime;
        }
        if (keyMap[sf::Keyboard::S]) {
            dest.y += 1000.0f * zoom * deltaTime;
        }
        if (keyMap[sf::Keyboard::D]) {
            dest.x += 1000.0f * zoom * deltaTime;
        }
        info.index = -1;
        sf::Vector2f deltaView = (dest - view.getCenter()) * 0.1f;
        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        view.setCenter(view.getCenter() + deltaView);
        window.setView(view);
        sf::Vector2f mousePos = window.mapPixelToCoords(mousePosI);
        
        window.clear(sf::Color::Black);
        
        for (int y = 0; y < MAXWIDTH; y++) {
            for (int x = 0; x < MAXWIDTH; x++) {
                sf::RectangleShape shape;
                sf::Vector2f pos(x * (WIDTH + WIDTH * 0.3f), y * (WIDTH + WIDTH * 0.3f));
                shape.setSize({ WIDTH, WIDTH });
                shape.setPosition(pos);
                shape.setFillColor(regular);
                sf::Vector2f intersectPos = mousePos;
                
                if (ptrMap[y][x] != -1) {
                    shape.setFillColor(node);
                    if (intersects(intersectPos, shape.getPosition())) {
                        shape.setFillColor(sf::Color::White);
                        info.index = ptrMap[y][x];
                        info.pos = pos;
                    }
                    Node *n = &nodes[ptrMap[y][x]];
                    if (n == firstNode || n == secondNode) {
                        shape.setFillColor(sf::Color::White);
                    }
                }
                window.draw(shape);
            }
        }
        
        if (path.first == 1) {
            int numPoints = (int) path.second.path.size();
            sf::VertexArray strip(sf::LineStrip, numPoints + 1);
            strip[0].color = sf::Color::White;
            strip[0].position = sf::Vector2f(firstNode->x * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f, firstNode->y * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f);
            for (int i = 0; i < path.second.path.size(); i++) {
                Node *n = path.second.path[i];
                strip[i + 1].color = sf::Color::White;
                strip[i + 1].position = sf::Vector2f(n->x * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f, n->y * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f);
            }
            window.draw(strip);
        }
        
        window.setView(window.getDefaultView());
        window.draw(title);
        
        if (info.index != -1) {
            sf::CircleShape circle;
            circle.setPosition(info.pos - sf::Vector2f(nodeRadius * (WIDTH + WIDTH * 0.3f), nodeRadius * (WIDTH + WIDTH * 0.3f)) + sf::Vector2f(WIDTH * 0.5f, WIDTH * 0.5f));
            circle.setFillColor(transparentNode);
            circle.setRadius(nodeRadius * (WIDTH + WIDTH * 0.3f));
            window.setView(view);
            window.draw(circle);
            
            Node &n = nodes[info.index];
            for (int i = 0; i < n.neighbors.size(); i++) {
                Node *neighbor = n.neighbors[i].second;
                sf::VertexArray lines(sf::LineStrip, 2);
                lines[0].position = sf::Vector2f(n.x * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f, n.y * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f);
                lines[0].color = lessTransparentNode;
                lines[1].position = sf::Vector2f(neighbor->x * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f, neighbor->y * (WIDTH + WIDTH * 0.3f) + WIDTH * 0.5f);
                lines[1].color = lessTransparentNode;
                window.draw(lines);
            }

            window.setView(window.getDefaultView());

            if (firstNode && secondNode && path.first == 0) {
                switch (type) {
                    case DV: {
                        std::pair<bool, Path *> info = hasRouteTo(firstNode, secondNode);
                        if (info.first) {
                            path.first = 1;
                            path.second = *info.second;
                        } else {
                            path.first = -1;
                        }
                        break;
                    }
                        
                    case LS: {
                        Path trace = pathfindLS(firstNode, secondNode);
                        if (trace.cost < 0.0f) {
                            path.first = -1;
                        } else {
                            path.first = 1;
                            path.second = trace;
                        }
                        break;
                    }
                }
            }
            
            if (showAnnoyingTag) {
                sf::RectangleShape rect;
                rect.setSize({ 400.0f, 200.0f });
                rect.setPosition(sf::Vector2f(mousePosI) + sf::Vector2f(20.0f, 20.0f));
                rect.setFillColor(node);
                rect.setOutlineThickness(10.0f);
                rect.setOutlineColor(node);
                window.draw(rect);
                rect.setFillColor(sf::Color::White);
                rect.setOutlineColor(sf::Color::White);
                rect.setPosition(sf::Vector2f(mousePosI) + sf::Vector2f(10.0f, 10.0f));
                window.draw(rect);
                
                std::wstring paths = L"";
                if (type == DV) {
                    paths = L"可用路径数量: " + std::to_wstring(nodes[info.index].routes.size());
                }
                std::wstring str = L"节点 " + std::to_wstring(info.index + 1) + L"\n" +
                                   L"邻居数量: " + std::to_wstring(nodes[info.index].neighbors.size()) + L"\n" +
                                   paths;
                sf::Text text(str, noto);
                text.setFillColor(sf::Color::Black);
                text.setPosition(rect.getPosition().x + 10.0f, rect.getPosition().y + 10.0f);
                window.draw(text);
            }
        }
        
        float dy = 0.0f;
        std::wstring errMsg = L"";
        if (path.first == -1) {
            dy = (size.y - 50.0f) - bottomBannerY;
            errMsg = L"找不到从节点 " + std::to_wstring(firstNode->id) + L" 到节点 " + std::to_wstring(secondNode->id) + L" 的路径";
            bannerColor = warning;
        } else if (path.first == 1) {
            dy = (size.y - 50.0f) - bottomBannerY;
            bannerColor = good;
            errMsg = L"整条路径花费为 " + std::to_wstring(path.second.cost) + L", 途径 " + std::to_wstring(path.second.path.size() - 1) + L" 个节点";
        } else {
            dy = (size.y + 10.0f) - bottomBannerY;
        }
        sf::RectangleShape shape;
        shape.setPosition(30.0f, bottomBannerY - 10.0f);
        shape.setSize(sf::Vector2f(size.x - 60.0f, 70.0f));
        shape.setFillColor(bannerColor);
        
        sf::Text error(errMsg, noto);
        error.setFillColor(sf::Color::White);
        error.setPosition(50.0f, bottomBannerY);
        window.draw(shape);
        window.draw(error);
        bottomBannerY += dy * 0.1f;
        
        gui.draw();
        window.display();
    }
    return 0;
}

#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>
#include <string>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const int GRID_SIZE = 50;
const int POINT_COUNT = 300;
const float INITIAL_RADIUS = 50.0f;
const float POINT_SPEED = 0.5f;

struct Point {
    float x, y;
    float vx, vy;

    Point(float x_ = 0, float y_ = 0, float vx_ = 0, float vy_ = 0)
        : x(x_), y(y_), vx(vx_), vy(vy_) {
    }
};

class SpatialHash {
private:
    std::unordered_map<int, std::vector<Point*>> hashTable;

    int hash(float x, float y) {
        int cellX = static_cast<int>(x) / GRID_SIZE;
        int cellY = static_cast<int>(y) / GRID_SIZE;
        return cellX * 73856093 ^ cellY * 19349663;
    }

public:
    void clear() {
        hashTable.clear();
    }

    void insert(Point* point) {
        int h = hash(point->x, point->y);
        hashTable[h].push_back(point);
    }

    std::vector<Point*> query(const Point& point) {
        int h = hash(point.x, point.y);
        std::vector<Point*> neighbors;
        for (int dx = -1; dx <= 1; ++dx) {
            for (int dy = -1; dy <= 1; ++dy) {
                int neighborHash = hash(point.x + dx * GRID_SIZE, point.y + dy * GRID_SIZE);
                if (hashTable.find(neighborHash) != hashTable.end()) {
                    neighbors.insert(neighbors.end(), hashTable[neighborHash].begin(), hashTable[neighborHash].end());
                }
            }
        }
        return neighbors;
    }
};

std::vector<Point> generateRandomPoints(int count) {
    std::vector<Point> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disX(0, WINDOW_WIDTH);
    std::uniform_real_distribution<> disY(0, WINDOW_HEIGHT);
    std::uniform_real_distribution<> disSpeed(-POINT_SPEED, POINT_SPEED);

    for (int i = 0; i < count; ++i) {
        points.push_back(Point(disX(gen), disY(gen), disSpeed(gen), disSpeed(gen)));
    }
    std::cout << "Generated " << points.size() << " points.\n";
    return points;
}

std::vector<Point*> findNeighborsWithinRadius(const Point& target, const std::vector<Point*>& candidates, float radius) {
    std::vector<Point*> neighbors;
    float radiusSquared = radius * radius;
    for (const auto& candidate : candidates) {
        float dx = candidate->x - target.x;
        float dy = candidate->y - target.y;
        if (dx * dx + dy * dy <= radiusSquared) {
            neighbors.push_back(candidate);
        }
    }
    return neighbors;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Spatial Hash Visualization");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Failed to load font!\n";
        return -1;
    }

    sf::Text text;
    text.setFont(font);
    text.setCharacterSize(14);
    text.setFillColor(sf::Color::White);

    std::vector<Point> points = generateRandomPoints(POINT_COUNT);
    SpatialHash spatialHash;

    float searchRadius = INITIAL_RADIUS;
    Point selectedPoint = { -1, -1 };
    std::vector<Point*> neighbors;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    selectedPoint = { static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y) };
                    auto candidates = spatialHash.query(selectedPoint);
                    neighbors = findNeighborsWithinRadius(selectedPoint, candidates, searchRadius);
                }
                if (event.mouseButton.button == sf::Mouse::Right) {
                    points.push_back(Point(static_cast<float>(event.mouseButton.x), static_cast<float>(event.mouseButton.y), 0, 0));
                    std::cout << "Added point at (" << event.mouseButton.x << ", " << event.mouseButton.y << ")\n";
                }
            }

            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Up) {
                    searchRadius += 5.0f;
                }
                else if (event.key.code == sf::Keyboard::Down) {
                    searchRadius = std::max(5.0f, searchRadius - 5.0f);
                }
            }
        }

        for (auto& point : points) {
            point.x += point.vx;
            point.y += point.vy;

            if (point.x < 0 || point.x > WINDOW_WIDTH) point.vx = -point.vx;
            if (point.y < 0 || point.y > WINDOW_HEIGHT) point.vy = -point.vy;
        }

        spatialHash.clear();
        for (auto& point : points) {
            spatialHash.insert(&point);
        }

        if (selectedPoint.x >= 0 && selectedPoint.y >= 0) {
            auto candidates = spatialHash.query(selectedPoint);
            neighbors = findNeighborsWithinRadius(selectedPoint, candidates, searchRadius);
        }

        window.clear(sf::Color::Black);

        for (int x = 0; x < WINDOW_WIDTH; x += GRID_SIZE) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(x, 0), sf::Color(100, 100, 100)),
                sf::Vertex(sf::Vector2f(x, WINDOW_HEIGHT), sf::Color(100, 100, 100))
            };
            window.draw(line, 2, sf::Lines);
        }
        for (int y = 0; y < WINDOW_HEIGHT; y += GRID_SIZE) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(0, y), sf::Color(100, 100, 100)),
                sf::Vertex(sf::Vector2f(WINDOW_WIDTH, y), sf::Color(100, 100, 100))
            };
            window.draw(line, 2, sf::Lines);
        }

        for (const auto& point : points) {
            sf::CircleShape shape(3);
            shape.setPosition(point.x, point.y);
            shape.setFillColor(sf::Color::White);
            window.draw(shape);
        }

        if (selectedPoint.x >= 0 && selectedPoint.y >= 0) {
            sf::CircleShape shape(5);
            shape.setPosition(selectedPoint.x, selectedPoint.y);
            shape.setFillColor(sf::Color::Red);
            window.draw(shape);

            for (const auto& neighbor : neighbors) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(selectedPoint.x, selectedPoint.y), sf::Color::Green),
                    sf::Vertex(sf::Vector2f(neighbor->x, neighbor->y), sf::Color::Green)
                };
                window.draw(line, 2, sf::Lines);

                sf::CircleShape neighborShape(4);
                neighborShape.setPosition(neighbor->x, neighbor->y);
                neighborShape.setFillColor(sf::Color::Blue);
                window.draw(neighborShape);
            }
        }

        text.setString("Points: " + std::to_string(points.size()) +
            "\nRadius: " + std::to_string(static_cast<int>(searchRadius)) +
            "\nNeighbors: " + std::to_string(neighbors.size()));
        window.draw(text);

        window.display();
    }

    return 0;
}

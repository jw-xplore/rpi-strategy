#pragma once
#include <vector>
#include <utility>
#include <queue>
#include <map>
#include <algorithm>

struct Vector2;
struct Node;
class World;

static int UNDISCOVERED_WEIGHT = 999;

struct Connection
{
	Node* fromNode;
	Node* node;
	float weight;
};

struct Node
{
	int x, y;
	//bool walkable = true;
	float travelCost;
	std::vector<Connection> connections;
		
	~Node() {};
};

enum ENodeRecordState
{
	Closed,
	Open,
	Unvisited
};

struct NodeRecordAs
{
	Node* node;
	Connection* connection;
	float costSoFar;
	float costEstimated;
	ENodeRecordState state;

	bool operator==(const NodeRecordAs& other) const {
		return node == other.node && costSoFar == other.costSoFar && costEstimated == other.costEstimated;
	}
};

struct NodeRecordAsCompare
{
	bool operator() (const NodeRecordAs& l, const NodeRecordAs& r) const { return l.costEstimated > r.costEstimated; }
};

class PathFinding
{
public:
	World* world;

	std::vector<Node> lastSearch;
	std::vector<Node*> nextUndiscovered;
	Node** nodes;
	int width, height;
	int visitsPerFrame = 100;

	PathFinding(World& world);
	~PathFinding();

	//void UpdateGraph(MapEntity* mapEntity);
	//void UpdateNode(int x, int y, bool walkable);
	void AddConnectionsToNode(Node* node, int x, int y);
	void DrawGraph();

	std::vector<Node>* AStarDivided(Vector2 start, Vector2 end, std::map<Node*, NodeRecordAs>& searchResult, std::priority_queue<NodeRecordAs, std::vector<NodeRecordAs>, NodeRecordAsCompare>& open);

	inline float ManhattanHeuristic(const Node* start, const Node* end);
	inline float ManhattanHeuristic(const int startX, const int startY, const int endX, const int endY);
	NodeRecordAs SmallestAsRecord(std::vector<NodeRecordAs>& list);
	bool ContainsAsRecord(const std::vector<NodeRecordAs>& list, Node* node, NodeRecordAs& outRecord);
	NodeRecordAs* FindAsRecordFromNode(std::vector<NodeRecordAs>& list, Node* node);
	void Discover(int x, int y);
	Node* ClosestUndiscovered(Vector2 pos);
};
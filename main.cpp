#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <unordered_map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <queue>
#include <utility>
#include <functional>

using namespace std;
struct OrganCost {
  int a;
  int b;
  int c;
  int d;
};

namespace Directions
{
    char positions2direct(pair<int, int> object, pair<int, int> source)
    {
        char direction = '0';
        if (object.second - source.second > 0)
        {
            direction = 'S';
        }
        else if (object.second - source.second < 0)
        {
            direction = 'N';
        }
        else if (object.first - source.first > 0)
        {
            direction = 'E';
        }
        else if (object.first - source.first < 0)
        {
            direction = 'W';
        }
        return direction;
    }
    pair<int, int> direct2position(char direction)
    {
        pair<int, int> retunred = { 0,0 };
        if (direction == 'N')
        {
           retunred = { 0,1 };
        }
        else if (direction == 'S')
        {
           retunred = { 0,-1 };
        }
        else if (direction == 'W')
        {
           retunred = { -1,0 };
        }
        else if (direction == 'E')
        {
           retunred = { 1,0 };
        }
        return retunred;
    }
}

unordered_map<string, OrganCost> organCosts = {{"BASIC", {1, 0, 0, 0}},
                                      {"HARVESTER", {0, 0, 1, 0}},
                                      {"TENTACLE", {0, 1, 1, 0}},
                                      {"SPORER", {0, 1, 0, 1}},
                                      {"ROOT", {1, 1, 1, 1}},
                                      {"FLY", {0,0,0,0}}};


struct Node {
  Node *parent;
  pair<int, int> position;
  OrganCost nodeCost;
  OrganCost pathCost;
  int g, h, f;
  pair<int, int> dir;
  Node(Node *parent = nullptr, pair<int, int> position = {0, 0})
      : parent(parent), position(position), g(0), h(0), f(0) {}

  bool operator==(const Node &other) const {
    return position == other.position;
  }
};

struct CompareNode {
  bool operator()(const Node *a, const Node *b) const { return a->f > b->f; }
};

struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto h1 = hash<T1>{}(p.first);
        auto h2 = hash<T2>{}(p.second);
        return h1 ^ (h2 << 1); // Combine hashes
    }
};

class Organ {
  public:
  pair<int, int> position;
  string type;
  int owner;
  int organ_id;
  string organ_dir;
  int organ_parent_id;
  int organ_root;
  int root_range;

  Organ(int x, int y, string type, int owner, int organ_id, string organ_dir,
        int organ_parent_id, int organ_root) {
    this->position.first = x;
    this->position.second = y;
    this->type = type;
    this->owner = owner;
    this->organ_id = organ_id;
    this->organ_dir = organ_dir;
    this->organ_parent_id = organ_parent_id;
    this->organ_root = organ_root;
    this->root_range = 0;
  }

  int calcAbsRange(shared_ptr<Organ> organ) {
    return abs(this->position.first - organ->position.first) +
           abs(this->position.second - organ->position.second);
  }
  bool operator==(const Organ &other) const {
    return this->position.first == other.position.first &&
           this->position.second == other.position.second &&
           this->type == other.type && this->organ_id == other.organ_id;
  }
  bool operator!=(const Organ &other) const {
    return this->position.first != other.position.first ||
           this->position.second != other.position.second ||
           this->type != other.type || this->organ_id != other.organ_id;
  }
  void print()
  {
    cerr << "position: " << position.first << " " << position.second
    << " type: " << type 
    << " owner: " << owner
    << " organ_id: " << organ_id
    << " organ_dir: " << organ_dir
    << " organ_parent_id: " << organ_parent_id
    << " organ_root: " << organ_root
    << " root_range: " << root_range
    << endl;
  } 
};

class GameBoard {
  public:
  int width;
  int height;
  vector<vector<int>> map;

  void setMapSize(int width, int height) {
    this->width = width;
    this->height = height;

    map.resize(width);
    for (auto &column : map) {
      column.resize(height);
      column.assign(height, 0);
    }
  }
  void printMap() {
    for (int i = 0; i < map[0].size(); i++) {
      for (auto column : map) {
        cerr << column[i] << " \t";
      }
      cerr << endl;
    }
  }
  int askForField(int x, int y) {
     return map.at(x).at(y); 
  }
  void setBoardField(shared_ptr<Organ> organ) {
    if (organ->owner == 0) {
      map.at(organ->position.first).at(organ->position.second) =
          -1 * organ->organ_id;
    } else if (organ->owner == 1) {
      map.at(organ->position.first).at(organ->position.second) =
          1 * organ->organ_id;
    } else {
      // map.at(organ->x).at(organ->y) = 1* organ-> organ_id;
      if (organ->type == "WALL") {
        map.at(organ->position.first).at(organ->position.second) =
            500 + organ->organ_id;
      } else if (organ->type == "A") {
        map.at(organ->position.first).at(organ->position.second) =
            501 + organ->organ_id;
      } else if (organ->type == "B") {
        map.at(organ->position.first).at(organ->position.second) =
            502 + organ->organ_id;
      } else if (organ->type == "C") {
        map.at(organ->position.first).at(organ->position.second) =
            503 + organ->organ_id;
      } else if (organ->type == "D") {
        map.at(organ->position.first).at(organ->position.second) =
            504 + organ->organ_id;
      }
    }
  }
  bool isNeighbourVal(int x, int y, int val) {
    if ((y - 1 >= 0 && (map.at(x).at(y - 1) < 0 || map.at(x).at(y - 1) == 500)) && (y + 1 < this->height && (map.at(x).at(y + 1) < 0 ||  map.at(x).at(y + 1) == 500)) &&
     (x - 1 >= 0 && (map.at(x - 1).at(y) == 500 ||  map.at(x - 1).at(y) < 0)) && (x + 1 < this->width && (map.at(x + 1).at(y) == 500 ||  map.at(x + 1).at(y)  < 0)))
     {
        cerr << "false" << endl;
        return false;
     }
    if ((y - 1 >= 0 && map.at(x).at(y - 1) == val) || (y + 1 < this->height && map.at(x).at(y + 1) == val) ||
     (x - 1 >= 0 && map.at(x - 1).at(y) == val) || (x + 1 < this->width && map.at(x + 1).at(y) == val))
     {
      return true;
     }
      if ((y - 1 >= 0 && map.at(x).at(y - 1) > 501) || (y + 1 < this->height && map.at(x).at(y + 1) > 501 ) ||
     (x - 1 >= 0 && map.at(x - 1).at(y) > 501) || (x + 1 < this->width && map.at(x + 1).at(y) > 501))
     {
      return true;
     }
    return false;
  }
  vector<pair<char, int>> askForNeigbour(int x, int y) {
    vector<pair<char, int>> neigbour;
    pair<char, int> N_id('N', 500);
    pair<char, int> S_id('S', 500);
    pair<char, int> W_id('W', 500);
    pair<char, int> E_id('E', 500);
    if (y - 1 >= 0)
      N_id.second = map.at(x).at(y - 1);
    if (y + 1 < this->height)
      S_id.second = map.at(x).at(y + 1);
    if (x - 1 >= 0)
      W_id.second = map.at(x - 1).at(y);
    if (x + 1 < this->width)
      E_id.second = map.at(x + 1).at(y);
    // cerr << "dirs " << N_id.second << " " << S_id.second << " " <<
    // W_id.second
    //      << " " << E_id.second << endl;
    neigbour.push_back(N_id);
    neigbour.push_back(S_id);
    neigbour.push_back(W_id);
    neigbour.push_back(E_id);
    return neigbour;
  }
};

class Protein : public Organ
{
  public:
  GameBoard distanceBoard;

  Protein(int x, int y, string type, int owner, int organ_id, string organ_dir,
        int organ_parent_id, int organ_root) : Organ(x, y, type, owner, organ_id, organ_dir,
        organ_parent_id, organ_root){}
  Protein(const shared_ptr<Organ> organ) : Organ(*organ){
    this->position = organ->position;
    this->type = organ->type;
    this->owner = organ->owner;
    this->organ_id = organ->organ_id;
    this->organ_dir = organ->organ_dir;
    this->organ_parent_id = organ->organ_parent_id;
    this->organ_root = organ->organ_root;
    this->root_range = organ->root_range;
  }
  void findDistances(vector<vector<int>>& grid) {

    int rows = grid.size();
    int cols = grid[0].size();

    // Directions for moving: right, down, left, up
    vector<pair<int, int>> directions = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };

    // Distance matrix initialized with -1
    vector<vector<int>> distances(rows, vector<int>(cols, -1));
    // BFS queue (stores x, y coordinates)
    queue<pair<int, int>> q;

    // Initialize BFS with the starting point
    int startX = this->position.first, 
    startY = this->position.second;
    q.push({ startX, startY });
    distances[startX][startY] = 0; // Distance to starting point is 0

    // BFS loop
    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        // Explore all possible directions
        for (auto [dx, dy] : directions) {
            int nx = x + dx;
            int ny = y + dy;

            // Check if the new position is valid
            if (nx >= 0 && ny >= 0 && nx < rows && ny < cols && (grid[nx][ny] == 0 || grid[nx][ny] != 500)  && distances[nx][ny] == -1) {
                distances[nx][ny] = distances[x][y] + 1; // Update distance
                q.push({ nx, ny }); // Enqueue the new position
            }
        }
    }
    distanceBoard.map = distances;
    // distanceBoard.printMap();
  }


};

class Resurces {
public:
  Resurces(){};
  int a;
  int b;
  int c;
  int d;

  void print() {
    cerr << "a: " << this->a << " b: " << this->b << " c: " << this->c
         << " d: " << this->d << endl;
  }
  bool canAffordOrgan(OrganCost cost) {
    bool returnVal;
    if (this->a >= cost.a && this->b >= cost.b && this->c >= cost.c &&
        this->d >= cost.d) {
      returnVal = true;
    } else {
      returnVal = false;
    }
    return returnVal;
  }
  char harvestPriority() {
    char priorityProtein = 'A';
    int minVal = min({this->a, this->b, this->c, this->d});
    // cerr << "minVAl " << minVal << endl;
    if (minVal == a)
      priorityProtein = 'A';
    else if (minVal == b)
      priorityProtein = 'B';
    else if (minVal == c)
      priorityProtein = 'C';
    else if (minVal == d)
      priorityProtein = 'D';
    return priorityProtein;
  }

};

class ResourcesManager
  {
    public:
    Resurces my;
    Resurces enemy;
    unordered_map<pair<int,int>, shared_ptr<Protein>, pair_hash> proteins;
    unordered_map<pair<int,int>, shared_ptr<Protein>, pair_hash> proteins_A;
    unordered_map<pair<int,int>, shared_ptr<Protein>, pair_hash> proteins_B;
    unordered_map<pair<int,int>, shared_ptr<Protein>, pair_hash> proteins_C;
    unordered_map<pair<int,int>, shared_ptr<Protein>, pair_hash> proteins_D;
    unordered_map<pair<int,int>, shared_ptr<Organ>, pair_hash> myHarvestProtein;
    unordered_map<pair<int,int>, shared_ptr<Organ>, pair_hash> enemnyHarvestProtein;  
    shared_ptr<GameBoard> board;

    ResourcesManager(shared_ptr<GameBoard> board)
    {
      this->board = board;
    }

  void addProitein(shared_ptr<Organ> organ_ptr)
  { 
    if (organ_ptr->type == "A")
    {
      shared_ptr<Protein> protien_ptr = make_shared<Protein>(organ_ptr);
      proteins_A[organ_ptr->position] = protien_ptr;
      proteins.insert({organ_ptr->position, protien_ptr});
    }
    else if (organ_ptr->type == "B")
    {
      shared_ptr<Protein> protien_ptr = make_shared<Protein>(organ_ptr);
      proteins_B[organ_ptr->position] = protien_ptr;
      proteins.insert({organ_ptr->position, protien_ptr});
    }
    else if (organ_ptr->type == "C")
    {
      shared_ptr<Protein> protien_ptr = make_shared<Protein>(organ_ptr);
      proteins_C[organ_ptr->position] = protien_ptr;
      proteins.insert({organ_ptr->position, protien_ptr});
    }
    else if (organ_ptr->type == "D")
    {
      shared_ptr<Protein> protien_ptr = make_shared<Protein>(organ_ptr);
      proteins_D[organ_ptr->position] = protien_ptr;
      proteins.insert({organ_ptr->position, protien_ptr});
    }

  }

  void addHarvest(pair<int,int> xy, string dir)
  {
    pair<int,int> dirs;
    if(dir == "N")
        dirs = {0,1};
    if(dir == "S")
        dirs = {0,-1};
    if(dir == "W")
        dirs = {-1,0};
    if(dir == "E")
        dirs = {1,0};
    xy.first += dirs.first;
    xy.second += dirs.second;
    if(proteins.find(xy) != proteins.end())
    {
      myHarvestProtein.insert({xy, proteins.at(xy)});
    }

  }

  bool harvestEngouh()
  {
    bool A = false;
    bool B = false;
    bool C = false;
    bool D = false;
    for (auto har: myHarvestProtein)
    {
      
      if (har.second->type == "A" )
      {
        A = true;
      }
      else if (har.second->type == "B" )
      {
        B = true;
      }
      else if (har.second->type == "C" )
      {
        C = true;
      }
      else if (har.second->type == "D" )
      {
        D = true;
      }
    }
    return A && B && C && D;
  }
  void clear()
  {
    myHarvestProtein.clear();
  }
};

class Game
{
  public:
  unordered_map<int,deque<pair<int,int>>> moveQue;
  shared_ptr<GameBoard> board = make_shared<GameBoard>();
  unordered_map<pair<int,int>, shared_ptr<Organ>, pair_hash> organs;
  unordered_map<int, unordered_map<int, shared_ptr<Organ>>> myOrgans;
  unordered_map<int, unordered_map<int, shared_ptr<Organ>>> enemyOrgans;
  set<int> rejectedOrgans;
  unordered_map<pair<int,int>, shared_ptr<Organ>, pair_hash> walls;
  shared_ptr<ResourcesManager> resourceManager = make_shared<ResourcesManager>(board);
  
  shared_ptr<Organ> toGrowOrgan;
  unordered_map<int,shared_ptr<Organ>> targetOrgan;
  bool defence;

  vector<int> myRootsId;
  pair<int, int> lastBlock;

  bool spore = false;
  deque<pair<int, int>> astar(const vector<vector<int>> &maze, pair<int, int> start, pair<int, int> end) 
  {
    if(start.first == end.first && start.second == end.second)
    {
      return {};
    }
    Node *start_node = new Node(nullptr, start);
    Node *end_node = new Node(nullptr, end);

    vector<Node *> open_list;
    vector<Node *> closed_list;

    open_list.push_back(start_node);

    while (!open_list.empty()) {
      // Get the current node with the lowest f
      auto current_iter =
          max_element(open_list.begin(), open_list.end(), CompareNode());
      Node *current_node = *current_iter;
      // cerr << "open_list.size() " << open_list.size() << endl;
      open_list.erase(current_iter);
      closed_list.push_back(current_node);

      // Check if goal is reached
      if (*current_node == *end_node) {
        deque<pair<int, int>> path;
        Node *current = current_node;
        while (current != nullptr) {
          path.push_front(current->position);
          current = current->parent;
        }

        path.pop_front();
        path.pop_back();
        return path;
      }
      // Generate children
      vector<string> types = {"BASIC", "SPORER"};
      vector<pair<int, int>> sporeDirections = {{0, -5}, {0, 5}, {-5, 0}, {5, 0},
      {0, -4}, {0, 4}, {-4, 0}, {4, 0},
      {0, -3}, {0, 3}, {-3, 0}, {3, 0},
      {0, -2}, {0, 2}, {-2, 0}, {2, 0},
      {0, -1}, {0, 1}, {-1, 0}, {1, 0}};
      vector<pair<int, int>> directions = {{0, -1}, {0, 1}, {-1, 0}, {1, 0}};
      vector<Node *> children;
      // cerr << ":nodeL: " << current_node->position.first << " " << current_node->position.second << endl;
      for (auto &dir : directions) {
        pair<int, int> node_position = {current_node->position.first + dir.first,
                                        current_node->position.second +
                                            dir.second};
        
        // Check if within maze bounds
        if (node_position.first < 0 || node_position.first >= maze.size() ||
            node_position.second < 0 || node_position.second >= maze[0].size()) {
          continue;
        }
        // Check if walkable
        if ((node_position != end_node->position && maze[node_position.first][node_position.second] > 0 && maze[node_position.first][node_position.second] < 501 )) {
          continue;
        }
        bool bad = false;
        vector<pair<char, int>> neighbours = board->askForNeigbour(node_position.first, node_position.second);
        // cerr << "neighbours " << neighbours.size() << endl;
          for(auto neighbour : neighbours)
          {
            // cerr << "DIRN: " << neighbour.second << " " << endl;
            if(neighbour.second < 0)
            {
              for(auto enemy : enemyOrgans)
              {
                // cerr << "DIRN1: " << neighbour.second << " " << endl;

                if(enemy.second.find(-1*neighbour.second) !=enemy.second.end())
                {
                  // cerr << "DIRF: " << enemy.second[-1*neighbour.second]->type << endl;
                  if(enemy.second[-1*neighbour.second]->type =="TENTACLE")
                  { 
                    char dir = Directions::positions2direct(node_position, enemy.second[-1*neighbour.second]->position);
                    // cerr << "DIR: " << dir << " " << enemy.second[-1*neighbour.second]->organ_dir << endl;
                    if(*enemy.second[-1*neighbour.second]->organ_dir.c_str() == dir)
                    {
                      bad = true;
                    }
                  }
                }
              }
            }
          }
        if (bad)
        {
          continue;
        }
        Node *new_node = new Node(current_node, node_position);
        new_node->dir = dir;
        children.push_back(new_node);
      }

      // cerr << "children" << children.size() << endl;
      for (auto *child : children) {
        // If child is already in closed list, skip it
        if (find_if(closed_list.begin(), closed_list.end(),
                    [&child](Node *closed_child) {
                      return *child == *closed_child;
                    }) != closed_list.end()) {
          delete child; // Clean up memory
          continue;
        }
        if(child->dir.first == current_node->dir.first && child->dir.second == current_node->dir.second)
        {
          child->g = current_node->g;
        }
        else
        {
          child->g = current_node->g + 1;
        }
        if(maze[child->position.first][child->position.second] < 0 && child->parent == start_node)
        {
          child->g += 10000;
        }
        if(maze[child->position.first][child->position.second] > 500 && child->position.first != end.first && child->position.second != end.second)
        {
          child->g += 100;
        }
        child->h = abs(child->position.first - end_node->position.first) +
                  abs(child->position.second - end_node->position.second);
        child->f = child->g + child->h;
        // If child is already in open list with a lower g, skip it
        auto open_iter =
            find_if(open_list.begin(), open_list.end(),
                    [&child](Node *open_node) { return *child == *open_node; });
        if (open_iter != open_list.end() && child->g > (*open_iter)->g){
          delete child; // Clean up memory
          continue;
        }

        // Add child to open list
        open_list.push_back(child);
        }
      }

      // If no path is found, return empty path
      return {};
    }

  void addOrgan(shared_ptr<Organ> organ_ptr)
  {
    if(organs.find(organ_ptr->position) == organs.end() || !(*organs.at(organ_ptr->position) == *organ_ptr) )
    {
      ///@todo delete killed organs
      organs[organ_ptr->position] = organ_ptr;
    
      if (organ_ptr->owner > 0) //My
      {
        if(organ_ptr->organ_parent_id)
        { 
          if(organ_ptr->type=="HARVESTER")
          { 
            resourceManager->addHarvest(organ_ptr->position, organ_ptr->organ_dir);
          }
          int parent_root_range = myOrgans.at(organ_ptr->organ_root).at(organ_ptr->organ_parent_id)->root_range;
          organ_ptr->root_range = parent_root_range + 1;
        }
        else if (organ_ptr->type=="ROOT")
        {
          // cerr << organ_ptr->organ_id << " BOOL " << !(*(organs.at(organ_ptr->position)) == *organ_ptr) << endl;

          myRootsId.push_back(organ_ptr->organ_id);
        }
        myOrgans[organ_ptr->organ_root].insert({organ_ptr->organ_id ,organ_ptr});
      }
      else if(organ_ptr->owner == 0) //Enemy
      {
        if(organ_ptr->organ_parent_id)
        {
          // organ_ptr->print();
          int parent_root_range = enemyOrgans.at(organ_ptr->organ_root).at(organ_ptr->organ_parent_id)->root_range;
          organ_ptr->root_range = parent_root_range+ 1;
        }
        // if(organ_ptr->type == "TENTACLE")
        // {
          // char dir = *(organ_ptr->organ_dir.c_str());
          // pair<int, int> pos = Directions::direct2position(dir);
          // cerr << "500: " << organ_ptr->position.first + pos.first << " " << organ_ptr->position.second + pos.second;
          // board->map[organ_ptr->position.first + pos.first][organ_ptr->position.second + pos.second] = 500;
        // }
        enemyOrgans[organ_ptr->organ_root].insert({organ_ptr->organ_id ,organ_ptr});
        for (auto root : myOrgans)
        {
          for (auto my : root.second)
          {
              if (organ_ptr->calcAbsRange(my.second) == 3)
              {
                cerr << "DANGER" << endl;
                defence = true;
                toGrowOrgan = my.second;
                targetOrgan[my.second->organ_root] = organ_ptr;
              }
              
          }
        }
      }
      else if(organ_ptr->type == "WALL") //Wall
      {
        walls[organ_ptr->position] = (organ_ptr);
      }
      else //Protein
      {
        resourceManager->addProitein(organ_ptr);
      }
    }
    board->setBoardField(organ_ptr);

  }
  void findTarget(int rootId){
    int tmpDistance = 100;
    int minDistance = 100;
    int val = 0;
    unordered_map<pair<int,int>, shared_ptr<Protein>, pair_hash> targets = resourceManager->proteins_D;
    char prior = resourceManager->my.harvestPriority();
    if (resourceManager->myHarvestProtein.size() >= 0) {
      if (prior == 'A') {
        targets = resourceManager->proteins_A;
      } else if (prior == 'B') {
        targets = resourceManager->proteins_B;
      } else if (prior== 'C') {
        targets = resourceManager->proteins_C;
      } else if (prior == 'D') {
        targets = resourceManager->proteins_D;
      }
    }
    for (auto organ_ptr : myOrgans.at(rootId))
    {
      if(board->isNeighbourVal(organ_ptr.second->position.first, organ_ptr.second->position.second, val))
      {
        if(rejectedOrgans.find(organ_ptr.second->organ_id) == rejectedOrgans.end())
        {
          for(auto posibleTarget_ptr : targets)
          {
            if(resourceManager->myHarvestProtein.find(posibleTarget_ptr.second->position) == resourceManager->myHarvestProtein.end())
            {
              tmpDistance = posibleTarget_ptr.second->distanceBoard.askForField(organ_ptr.second->position.first, organ_ptr.second->position.second);
              // cerr << "distance" << tmpDistance << " " << minDistance << endl;
              if(tmpDistance < minDistance && tmpDistance > 1 && board->askForField(posibleTarget_ptr.first.first, posibleTarget_ptr.first.second) > 500)
              {
                // posibleTarget_ptr.second->distanceBoard.printMap();
                minDistance = tmpDistance;
                toGrowOrgan = organ_ptr.second;
                targetOrgan[rootId] = posibleTarget_ptr.second;
                // cerr << "found: " << rootId << endl;
                // toGrowOrgan->print();
                // targetOrgan[rootId]->print();
              }
            }
          }
        }

      }
    }
    if(minDistance == 100)
    {
      fight(rootId);
    }
  }
  void fight(int rootId)
  {
        int tmpDistance = 100;
    int minDistance = 100;
    int val = 0;

    for (auto organ_ptr : myOrgans.at(rootId))
    {
      if(board->isNeighbourVal(organ_ptr.second->position.first, organ_ptr.second->position.second, val))
      {
        if(rejectedOrgans.find(organ_ptr.second->organ_id) == rejectedOrgans.end())
        {
          // for(auto posibleTarget_ptr : enemyOrgans.begin()->second)
          // {
            // if(targetOrgan[rootId] == nullptr)
            // {
              tmpDistance = enemyOrgans.begin()->second.begin()->second->calcAbsRange(organ_ptr.second);
            // }
            // else 
            // {
            //   tmpDistance = targetOrgan[rootId]->calcAbsRange(organ_ptr.second);
            // }
              if(tmpDistance < minDistance && tmpDistance > 0)
              {
                // cerr << "distance" << tmpDistance << " " << minDistance << endl;

                // posibleTarget_ptr.second->distanceBoard.printMap();
                minDistance = tmpDistance;
                toGrowOrgan = organ_ptr.second;
                targetOrgan[rootId] = enemyOrgans.begin()->second.begin()->second;
              }
            }
          // }
        }
      }
      if(minDistance < 100)
      {
        cerr << "FIGHT: " << rootId << endl;
        toGrowOrgan->print();
        targetOrgan[rootId]->print();
      }
    }
  void planing() {
    pair<int, int> start = toGrowOrgan->position;
    pair<int, int> stop = targetOrgan[toGrowOrgan->organ_root]->position;
    cerr << toGrowOrgan->position.first << " " << toGrowOrgan->position.second << " planing for: " << targetOrgan[toGrowOrgan->organ_root]->position.first << " " << targetOrgan[toGrowOrgan->organ_root]->position.second << endl;;
    // boardId.printMap();
    moveQue[toGrowOrgan->organ_root] = astar(board->map, start, stop);
    if (!moveQue[toGrowOrgan->organ_root].empty()) {
      cerr << "Path found:\n";
      cerr << moveQue[toGrowOrgan->organ_root].size() << endl;
      for (auto &position : moveQue[toGrowOrgan->organ_root]) {
        cerr << "(" << position.first << ", " << position.second << ") ";
      }
      cerr << endl;
    } else {
      cerr << "No pathd." << endl;
    }
  }
  void chooseType(pair<int, int> xy, string &type, string &direction){
    type = "BASIC";
    if(!(resourceManager->my.canAffordOrgan(organCosts["BASIC"])) || defence)
    {
      if(resourceManager->my.canAffordOrgan(organCosts["TENTACLE"]) && (defence || (resourceManager->my.c >= resourceManager->my.d || !(resourceManager->my.canAffordOrgan(organCosts["SPORER"])))))
      {
        cerr << "TENTACLE?? " << endl;
        type = "TENTACLE";
      }
      else if(resourceManager->my.canAffordOrgan(organCosts["SPORER"]) && !defence)
      {
        type = "SPORER";
      }
      else if(resourceManager->my.canAffordOrgan(organCosts["HARVESTER"]) && !defence)
      {
        type = "HARVESTER";
      }
    }
    vector<pair<char, int>> neighbours = board->askForNeigbour(xy.first, xy.second);
    for (auto p : neighbours)
    {
      // cerr << "HARVESTER? " << p.second << " " << xy.first << " " << xy.second<< endl;
      if(p.second > 500)
      {
        if(moveQue[toGrowOrgan->organ_root].size() > 1 && resourceManager->my.c > 1 && resourceManager->my.d > 1 && resourceManager->my.canAffordOrgan(organCosts["HARVESTER"]))
        {
          type = "HARVESTER";
          direction = p.first;
        }
        else if (moveQue[toGrowOrgan->organ_root].size() == 1  && resourceManager->my.canAffordOrgan(organCosts["HARVESTER"]))
        {
          type = "HARVESTER";
          direction = p.first;
        }
      }
      if(p.second < 0  && resourceManager->my.canAffordOrgan(organCosts["TENTACLE"]))
      {
        cerr << "TENTACLE?? XD " << endl;
        type = "TENTACLE";
        direction = p.first;
        break;
      }
    }
    if(moveQue[toGrowOrgan->organ_root].size() > 4 && resourceManager->my.d > 5 && toGrowOrgan->type != "SPORER" && type == "BASIC" && resourceManager->my.canAffordOrgan(organCosts["SPORER"]))
    {
      if(moveQue[toGrowOrgan->organ_root][0].first == moveQue[toGrowOrgan->organ_root][2].first ||
       moveQue[toGrowOrgan->organ_root][0].second == moveQue[toGrowOrgan->organ_root][2].second)
      {
        type = "SPORER";
        if(moveQue[toGrowOrgan->organ_root][0].first - moveQue[toGrowOrgan->organ_root][2].first > 0)
          direction = "W";
        else if (moveQue[toGrowOrgan->organ_root][0].first - moveQue[toGrowOrgan->organ_root][2].first < 0)
          direction = "E";
        else if (moveQue[toGrowOrgan->organ_root][0].second - moveQue[toGrowOrgan->organ_root][2].second > 0)
          direction = "N";
        else 
          direction = "S";
        spore = true;
      }
    }
  }
  void getNextMove(string &moveType, int &organ_id, int &toMoveX, int &toMoveY,
                   string &type, string &direction) {
    if(moveQue[toGrowOrgan->organ_root].empty())
    {
      rejectedOrgans.insert(toGrowOrgan->organ_id);
    }
    else
    {
      rejectedOrgans.clear();
    }
    if(targetOrgan[toGrowOrgan->organ_root] == nullptr)
    {
      cerr << "NULLL" << endl;
    }
    bool error = false;
    if (spore && toGrowOrgan->type == "SPORER")
    {
      moveType = "SPORE";
      int x = moveQue[toGrowOrgan->organ_root].front().first;
      int y = moveQue[toGrowOrgan->organ_root].front().second;
      while((moveQue[toGrowOrgan->organ_root][1].first == x || moveQue[toGrowOrgan->organ_root][1].second == y) && moveQue[toGrowOrgan->organ_root].size() > 3)
      {
        moveQue[toGrowOrgan->organ_root].pop_front();
      }
      spore = false;
    }
    else
    {
      moveType = "GROW";
    }
      if (moveQue[toGrowOrgan->organ_root].size()) {
        // organ_id = board->askForField(lastBlock.first, lastBlock.second);
        // if (organ_id <= 0) {
          organ_id = toGrowOrgan->organ_id;
        // }
        toMoveX = moveQue[toGrowOrgan->organ_root].front().first;
        toMoveY = moveQue[toGrowOrgan->organ_root].front().second;
        chooseType(moveQue[toGrowOrgan->organ_root].front(), type, direction);

        lastBlock = moveQue[toGrowOrgan->organ_root].front();
        moveQue[toGrowOrgan->organ_root].pop_front();
      }
      else
      {
        vector<pair<char, int>>  neighbours = board->askForNeigbour(toGrowOrgan->position.first, toGrowOrgan->position.second);
        for (auto neighbour : neighbours )
        {
          if(neighbour.second > 500 || neighbour.second == 0)
          {
            pair<int, int> xy = wayToXY(neighbour.first);
            toMoveX = toGrowOrgan->position.first + xy.first;
            toMoveY = toGrowOrgan->position.second + xy.second;
            chooseType({toMoveX, toMoveY}, type, direction);
            organ_id = toGrowOrgan->organ_id;
          }
        }
      }
      if(resourceManager->myHarvestProtein.find({toMoveX, toMoveY}) != resourceManager->myHarvestProtein.end())
      {
        resourceManager->myHarvestProtein.erase({toMoveX, toMoveY});
      }
    cerr << "rej: " <<  rejectedOrgans.size() << endl;;
  }
  pair<int, int> wayToXY(char way) {
    pair<int, int> xy = {0, 0};
    switch (way) {
    case 'N':
      xy.first = 0;
      xy.second = -1;
      break;
    case 'S':
      xy.first = 0;
      xy.second = 1;
      break;
    case 'W':
      xy.first = -1;
      xy.second = 0;
      break;
    case 'E':
      xy.first = 1;
      xy.second = 0;
      break;
    }
    return xy;
  }
  void checkMyOgans()
  {
    vector<shared_ptr<Organ>> toDelete;
    for(auto &root : myOrgans)
    {
      for(auto &organ : root.second)
      {
        if(board->askForField(organ.second->position.first, organ.second->position.second) < 1)
        {
          toDelete.push_back(organ.second);
        }
      }
      for(auto &del : toDelete)
      {
          // cerr << "delete my " << root.second.size() << endl;
          root.second.erase(del->organ_id);
          // del->print();
          del.reset();
      } 
      toDelete.clear();
    }
    for(auto &root : enemyOrgans)
    {
      for(auto &organ : root.second)
      {
        if(board->askForField(organ.second->position.first, organ.second->position.second) >= 0)
        {
          toDelete.push_back(organ.second);
        }
      }
      for(auto &del : toDelete)
      {
          // cerr << "delete" << endl;
          root.second.erase(del->organ_id);
          // del->print();
          del.reset();
      } 
      toDelete.clear();
    }
    for (auto it = enemyOrgans.begin(); it != enemyOrgans.end(); ) {
        if (it->second.empty()) { // Example condition: remove even keys
            it = enemyOrgans.erase(it); // Erase returns the next iterator
        } else {
            ++it; // Move to the next element
        }
    }
    for (auto it = myOrgans.begin(); it != myOrgans.end(); ) {
        if (it->second.empty()) { // Example condition: remove even keys
            it = myOrgans.erase(it); // Erase returns the next iterator
        } else {
            ++it; // Move to the next element
        }
    }
  }
  void clearArray() {
    moveQue.clear();
    organs.clear();
    myOrgans.clear();
    enemyOrgans.clear();
    rejectedOrgans.clear();
    resourceManager->clear();
  }
};

bool firstMove = true;

int main()
{
  // GameBoard board;
    int width; // columns in the game grid
    int height; // rows in the game grid
    cin >> width >> height; cin.ignore();
    
    // board.setMapSize(width, height);
    Game game;
    game.board->setMapSize(width, height);
    // game loop
    while (1) {
        game.targetOrgan.clear();
        int entity_count;
        cin >> entity_count; cin.ignore();
        for (int i = 0; i < entity_count; i++) {
            int x;
            int y; // grid coordinate
            string type; // WALL, ROOT, BASIC, TENTACLE, HARVESTER, SPORER, A, B, C, D
            int owner; // 1 if your organ, 0 if enemy organ, -1 if neither
            int organ_id; // id of this entity if it's an organ, 0 otherwise
            string organ_dir; // N,E,S,W or X if not an organ
            int organ_parent_id;
            int organ_root_id;
            cin >> x >> y >> type >> owner >> organ_id >> organ_dir >> organ_parent_id >> organ_root_id; cin.ignore();
            shared_ptr<Organ> organ_ptr = make_shared<Organ>(x,y,type,owner,organ_id,organ_dir,organ_parent_id,organ_root_id);
            game.addOrgan(organ_ptr);
        }
        // game.board->printMap();
        if(firstMove)
        {
          for( auto a : game.resourceManager->proteins)
          {
            a.second->findDistances(game.board->map);
            // a->distanceBoard.printMap();
          }
        }
 
        cin >> game.resourceManager->my.a >> game.resourceManager->my.b >> game.resourceManager->my.c >> game.resourceManager->my.d; cin.ignore();
        cin >> game.resourceManager->enemy.a >> game.resourceManager->enemy.b >> game.resourceManager->enemy.c >> game.resourceManager->enemy.d; cin.ignore();
        game.resourceManager->enemy.print();
        int required_actions_count; // your number of organisms, output an action for each one in any order
        cin >> required_actions_count; cin.ignore();
        // game.board->printMap();
        game.checkMyOgans();
        game.myRootsId.clear();
        for (auto a : game.myOrgans)
        {
          game.myRootsId.push_back(a.first);
        }
        for (int i = 0; i < required_actions_count; i++) {

            // Write an action using cout. DON'T FORGET THE "<< endl"
            // To debug: cerr << "Debug messages..." << endl;
            
            cerr << endl << "ROOT: " << game.myRootsId[i] << endl;
            for(auto a : game.myRootsId)
            {
              cerr << a << " ";
            }
            cerr << endl;
            if(game.targetOrgan[game.myRootsId[i]] == nullptr)
            {
              if(game.resourceManager->myHarvestProtein.size() > 3 && game.resourceManager->harvestEngouh())
              { 
                cerr << "fight " <<endl;
                game.fight(game.myRootsId[i]);
                cerr << "planing " <<endl;
                if(game.targetOrgan[game.myRootsId[i]].use_count() > 0)
                {
                  cerr << "planing " << game.targetOrgan[game.myRootsId[i]]->position.first << " " << game.targetOrgan[game.myRootsId[i]]->position.second << endl;
                  game.planing();
                }
              }
              else
              {
                cerr << "find Target " <<endl;
                game.findTarget(game.myRootsId[i]);
                cerr << "planing " << game.targetOrgan[game.myRootsId[i]].use_count() <<endl;
                if(game.targetOrgan[game.myRootsId[i]].use_count() > 0)
                {
                  cerr << "planing2 " << game.targetOrgan[game.myRootsId[i]]->position.first << " " << game.targetOrgan[game.myRootsId[i]]->position.second << endl;
                  game.planing();
                }
              }
            }
            else
            {
              cerr << "planing def" <<endl;
              game.fight(game.myRootsId[i]);
              cerr << "planing def" << game.targetOrgan[game.myRootsId[i]]->position.first << " " << game.targetOrgan[game.myRootsId[i]]->position.second << endl;
              game.planing();
            }
            
            string moveType;
            int organ_id = 0;
            int toMoveX;
            int toMoveY;
            string type;
            string direction;
            game.getNextMove(moveType, organ_id, toMoveX, toMoveY, type,
                                   direction);
            if (organ_id > 0) {
              cout << moveType + " " + to_string(organ_id) + " " + to_string(toMoveX) + " " + to_string(toMoveY) + " " + type + " " + direction << endl;
            } else {
              cout << "WAIT" << endl;
            }
            
        }
        firstMove = false;
        for (auto& row : game.board->map) {
          for (auto& val : row) {
            val = 0;
          }
        }
    }
}
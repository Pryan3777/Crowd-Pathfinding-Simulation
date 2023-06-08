// Custom struct to hold NavNode information for A* navigation
struct FNavNodeInfo
{
    class ANavNode* _NavNode;

    // Add additional properties as needed for A* navigation, e.g., cost, heuristic, parent node, etc.
    float _Cost;
    float _Heuristic;
    FNavNodeInfo* _ParentNode;

    // Constructor
    FNavNodeInfo(class ANavNode* NavNode, float Cost, float Heuristic, FNavNodeInfo* ParentNode);
};


// Custom struct to hold NavNode information for A* calculation
struct FNavNodeInfo
{
    class ANavNode* _NavNode;

    float _Cost;
    float _Heuristic;
    FNavNodeInfo* _ParentNode;

    FNavNodeInfo(class ANavNode* NavNode, float Cost, float Heuristic, FNavNodeInfo* ParentNode);
};


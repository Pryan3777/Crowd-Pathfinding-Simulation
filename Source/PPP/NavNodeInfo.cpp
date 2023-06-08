// Fill out your copyright notice in the Description page of Project Settings.


#include "NavNodeInfo.h"

FNavNodeInfo::FNavNodeInfo(ANavNode* NavNode, float Cost, float Heuristic, FNavNodeInfo* ParentNode) :
	_NavNode(NavNode),
	_Cost(Cost),
	_Heuristic(Heuristic),
	_ParentNode(ParentNode)
{
}

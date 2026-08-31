// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Growth/PCGExValencyGrowthBFS.h"

#pragma region FPCGExValencyGrowthBFS

int32 FPCGExValencyGrowthBFS::SelectNextConnector(TArray<FPCGExOpenConnector>& Frontier)
{
	if (Frontier.IsEmpty()) { return INDEX_NONE; }

	// BFS: lowest depth first; break ties by highest priority
	int32 BestIdx = 0;
	for (int32 i = 1; i < Frontier.Num(); ++i)
	{
		const FPCGExOpenConnector& Curr = Frontier[i];
		const FPCGExOpenConnector& Best = Frontier[BestIdx];
		if (Curr.Depth < Best.Depth ||
			(Curr.Depth == Best.Depth && Curr.Priority > Best.Priority))
		{
			BestIdx = i;
		}
	}
	return BestIdx;
}

#pragma endregion

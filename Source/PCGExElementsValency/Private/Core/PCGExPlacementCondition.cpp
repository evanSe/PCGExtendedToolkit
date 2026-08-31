// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Core/PCGExPlacementCondition.h"

#include "Growth/PCGExValencyGenerativeCommon.h"

#pragma region FPCGExPlacementCondition_BoundsOverlap

bool FPCGExPlacementCondition_BoundsOverlap::Evaluate(const FPCGExPlacementContext& Context) const
{
	return !Context.WorldBounds.IsValid
		|| !Context.BoundsTracker
		|| !Context.BoundsTracker->Overlaps(Context.WorldBounds);
}

#pragma endregion

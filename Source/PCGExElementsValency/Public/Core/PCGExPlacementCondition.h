// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"

#include "PCGExPlacementCondition.generated.h"

class FPCGExBoundsTracker;
struct FPCGExValencyBondingRulesCompiled;

/**
 * Context passed to placement condition evaluation during growth.
 */
struct PCGEXELEMENTSVALENCY_API FPCGExPlacementContext
{
	FBox WorldBounds = FBox(ForceInit);
	FTransform WorldTransform = FTransform::Identity;
	int32 ModuleIndex = -1;
	int32 Depth = 0;
	float CumulativeWeight = 0.0f;
	int32 PlacedCount = 0;
	const FPCGExBoundsTracker* BoundsTracker = nullptr;
	const FPCGExValencyBondingRulesCompiled* CompiledRules = nullptr;
};

/**
 * Base placement condition — override Evaluate() to implement custom validation.
 * All conditions in a module's stack must pass for placement to succeed.
 */
USTRUCT(BlueprintType)
struct PCGEXELEMENTSVALENCY_API FPCGExPlacementCondition
{
	GENERATED_BODY()
	virtual ~FPCGExPlacementCondition() = default;

	/** Return true if placement is valid */
	virtual bool Evaluate(const FPCGExPlacementContext& Context) const { return true; }
};

/**
 * Rejects placement if world bounds overlap any existing placed module.
 */
USTRUCT(BlueprintType, DisplayName = "Bounds Overlap")
struct PCGEXELEMENTSVALENCY_API FPCGExPlacementCondition_BoundsOverlap : public FPCGExPlacementCondition
{
	GENERATED_BODY()

	virtual bool Evaluate(const FPCGExPlacementContext& Context) const override;
};

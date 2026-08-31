// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_Probability.generated.h"

/**
 * Filter constraint: randomly rejects a fraction of placement variants.
 * Uses a deterministic hash of the candidate transform and growth context
 * so results are reproducible for the same placement scenario.
 */
USTRUCT(BlueprintType, DisplayName="✕ · Probability")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_Probability : public FPCGExConstraintFilter
{
	GENERATED_BODY()

	/** Probability of keeping each variant (0 = reject all, 1 = keep all) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0, ClampMax = 1))
	float Chance = 0.5f;

	virtual bool IsValid(
		const FPCGExConstraintContext& Context,
		const FTransform& CandidateTransform) const override
	{
		// Deterministic pseudo-random from transform + context
		const FVector Pos = CandidateTransform.GetTranslation();
		const uint32 PosHash = HashCombine(
			GetTypeHash(Pos.X),
			HashCombine(GetTypeHash(Pos.Y), GetTypeHash(Pos.Z)));
		const uint32 CtxHash = HashCombine(
			GetTypeHash(Context.Depth),
			GetTypeHash(Context.ChildModuleIndex));
		const uint32 FinalHash = HashCombine(PosHash, CtxHash);
		const float Roll = static_cast<float>(FinalHash & 0xFFFF) / 65535.0f;
		return Roll <= Chance;
	}
};

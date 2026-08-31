// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"
#include "Math/PCGExMathAxis.h"

#include "PCGExConstraint_AlignToWorld.generated.h"

/**
 * Common world direction presets for alignment.
 */
UENUM(BlueprintType)
enum class EPCGExWorldDirection : uint8
{
	WorldUp      UMETA(DisplayName = "World Up (+Z)"),
	WorldDown    UMETA(DisplayName = "World Down (-Z)"),
	WorldForward UMETA(DisplayName = "World Forward (+X)"),
	WorldRight   UMETA(DisplayName = "World Right (+Y)"),
	Custom       UMETA(DisplayName = "Custom Direction")
};

/**
 * Modifier constraint: forces a local axis of the placed module to align with a world direction.
 * Critical for architectural elements that must remain upright regardless of parent orientation.
 */
USTRUCT(BlueprintType, DisplayName="○ · Align to World")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_AlignToWorld : public FPCGExConstraintModifier
{
	GENERATED_BODY()

	/** Which local axis of the placed module to align */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	EPCGExAxis LocalAxis = EPCGExAxis::Up;

	/** World direction to align toward */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	EPCGExWorldDirection Direction = EPCGExWorldDirection::WorldUp;

	/** Custom world direction (only used when Direction = Custom) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (EditCondition = "Direction==EPCGExWorldDirection::Custom"))
	FVector CustomDirection = FVector::UpVector;

	/** Blend strength (0 = no effect, 1 = full alignment) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0, ClampMax = 1))
	float Strength = 1.0f;

	virtual void ApplyModification(
		const FPCGExConstraintContext& Context,
		FTransform& InOutTransform,
		FRandomStream& Random) const override
	{
		FVector TargetDir;
		switch (Direction)
		{
		case EPCGExWorldDirection::WorldUp:      TargetDir = FVector::UpVector;       break;
		case EPCGExWorldDirection::WorldDown:     TargetDir = -FVector::UpVector;      break;
		case EPCGExWorldDirection::WorldForward:  TargetDir = FVector::ForwardVector;  break;
		case EPCGExWorldDirection::WorldRight:    TargetDir = FVector::RightVector;    break;
		case EPCGExWorldDirection::Custom:        TargetDir = CustomDirection.GetSafeNormal(); break;
		default:                                  TargetDir = FVector::UpVector;       break;
		}

		if (TargetDir.IsNearlyZero()) { return; }

		// Get current local axis direction from the transform
		const FVector CurrentDir = PCGExMath::GetDirection(InOutTransform.GetRotation(), LocalAxis);

		// Blend toward target
		const FVector BlendedDir = FMath::Lerp(CurrentDir, TargetDir, Strength).GetSafeNormal();
		if (BlendedDir.IsNearlyZero()) { return; }

		// Compute correction rotation
		const FQuat Correction = FQuat::FindBetweenNormals(CurrentDir, BlendedDir);
		InOutTransform.SetRotation(Correction * InOutTransform.GetRotation());
	}
};

// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_ArcSurface.generated.h"

/**
 * Modifier constraint: slides the attachment point to a random position within
 * an arc-shaped area on the connector face plane.
 * Combines angular range with inner/outer radius to produce sectors, rings,
 * annular arcs, full disks, or point-on-circle configurations.
 */
USTRUCT(BlueprintType, DisplayName="○ · Arc Surface")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_ArcSurface : public FPCGExConstraintModifier
{
	GENERATED_BODY()

	/** Center of the angular range in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = -360, ClampMax = 360))
	float CenterAngleDegrees = 0.0f;

	/** Half-width of the angular range in degrees (total sweep = 2 * HalfWidth). 180 = full circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0, ClampMax = 180))
	float HalfWidthDegrees = 180.0f;

	/** Inner radius of the arc area (0 = includes center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	float InnerRadius = 0.0f;

	/** Outer radius of the arc area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	float OuterRadius = 10.0f;

	float GetMinAngle() const { return CenterAngleDegrees - HalfWidthDegrees; }
	float GetMaxAngle() const { return CenterAngleDegrees + HalfWidthDegrees; }

	virtual void ApplyModification(
		const FPCGExConstraintContext& Context,
		FTransform& InOutTransform,
		FRandomStream& Random) const override
	{
		const FQuat ConnectorRot = Context.ParentConnectorWorld.GetRotation();
		const FVector Right = ConnectorRot.GetRightVector();
		const FVector Up = ConnectorRot.GetUpVector();

		// Random angle within the arc
		const float MinAngle = GetMinAngle();
		const float MaxAngle = GetMaxAngle();
		const float Angle = FMath::DegreesToRadians(MinAngle + Random.FRand() * (MaxAngle - MinAngle));

		// Random radius (uniform area distribution in annulus)
		const float R1Sq = InnerRadius * InnerRadius;
		const float R2Sq = OuterRadius * OuterRadius;
		const float EffectiveOuter = FMath::Max(InnerRadius, OuterRadius);
		const float EffR2Sq = EffectiveOuter * EffectiveOuter;
		const float Radius = (EffR2Sq > R1Sq)
			? FMath::Sqrt(R1Sq + Random.FRand() * (EffR2Sq - R1Sq))
			: InnerRadius;

		const FVector Offset = Right * (Radius * FMath::Cos(Angle)) + Up * (Radius * FMath::Sin(Angle));
		InOutTransform.AddToTranslation(Offset);
	}
};

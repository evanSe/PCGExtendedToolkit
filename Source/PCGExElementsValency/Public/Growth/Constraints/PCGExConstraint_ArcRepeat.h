// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_ArcRepeat.generated.h"

/**
 * Generator constraint: places evenly-spaced variants along an arc at a given radius.
 * Creates dash-like patterns around circles, sectors, or full rings.
 * Each variant is both positioned on the arc and rotated to follow it.
 */
USTRUCT(BlueprintType, DisplayName="● · Arc Repeat")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_ArcRepeat : public FPCGExConstraintGenerator
{
	GENERATED_BODY()

	/** Center of the angular range in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = -360, ClampMax = 360))
	float CenterAngleDegrees = 0.0f;

	/** Half-width of the angular range in degrees (total sweep = 2 * HalfWidth). 180 = full circle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0, ClampMax = 180))
	float HalfWidthDegrees = 180.0f;

	/** Radius from the connector center */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	float Radius = 10.0f;

	/** Number of evenly-spaced positions along the arc */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 1))
	int32 Steps = 6;

	/** Add random angular offset per step for variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	bool bRandomOffset = false;

	float GetMinAngle() const { return CenterAngleDegrees - HalfWidthDegrees; }
	float GetMaxAngle() const { return CenterAngleDegrees + HalfWidthDegrees; }

	virtual int32 GetMaxVariants() const override { return Steps; }

	virtual void GenerateVariants(
		const FPCGExConstraintContext& Context,
		FRandomStream& Random,
		TArray<FTransform>& OutVariants) const override
	{
		const FQuat ConnectorRot = Context.ParentConnectorWorld.GetRotation();
		const FVector Forward = ConnectorRot.GetForwardVector();
		const FVector Right = ConnectorRot.GetRightVector();
		const FVector Up = ConnectorRot.GetUpVector();

		const float MinAngle = GetMinAngle();
		const float Range = HalfWidthDegrees * 2.0f;
		const float StepSize = (Steps > 1) ? (Range / static_cast<float>(Steps)) : 0.0f;

		for (int32 i = 0; i < Steps; ++i)
		{
			float Angle = MinAngle + StepSize * static_cast<float>(i);
			if (bRandomOffset)
			{
				Angle += Random.FRand() * StepSize;
			}

			const float AngleRad = FMath::DegreesToRadians(Angle);

			// Position on the arc
			const FVector RadialOffset = Right * (Radius * FMath::Cos(AngleRad)) + Up * (Radius * FMath::Sin(AngleRad));

			// Rotation to follow the arc (spin around Forward)
			const FQuat ArcRotation(Forward, AngleRad);

			FTransform Variant = Context.BaseAttachment;
			Variant.AddToTranslation(RadialOffset);
			Variant.SetRotation(ArcRotation * Variant.GetRotation());

			OutVariants.Add(Variant);
		}
	}
};

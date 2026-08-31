// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_Spiral.generated.h"

/**
 * Generator constraint: places variants along a helix (spiral staircase pattern).
 * Combines angular rotation with progressive offset along the connector forward axis.
 * Each variant is both positioned on the helix and rotated to follow it.
 */
USTRUCT(BlueprintType, DisplayName="● · Spiral")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_Spiral : public FPCGExConstraintGenerator
{
	GENERATED_BODY()

	/** Total rotation in degrees across all steps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	float TotalRotationDegrees = 360.0f;

	/** Height gained per full 360-degree revolution (along connector forward) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	float HeightPerRevolution = 30.0f;

	/** Radius of the helix (distance from connector center) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	float Radius = 10.0f;

	/** Number of placement positions along the helix */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 1))
	int32 Steps = 8;

	/** Add random angular offset per step for variation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	bool bRandomOffset = false;

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

		const float StepAngle = (Steps > 1)
			? TotalRotationDegrees / static_cast<float>(Steps - 1)
			: 0.0f;
		const float HeightPerDegree = (FMath::Abs(360.0f) > SMALL_NUMBER)
			? HeightPerRevolution / 360.0f
			: 0.0f;

		for (int32 i = 0; i < Steps; ++i)
		{
			float Angle = StepAngle * static_cast<float>(i);
			if (bRandomOffset)
			{
				Angle += Random.FRand() * StepAngle;
			}

			const float AngleRad = FMath::DegreesToRadians(Angle);
			const float Height = Angle * HeightPerDegree;

			// Radial offset on the plane perpendicular to Forward
			const FVector RadialOffset = Right * (Radius * FMath::Cos(AngleRad))
				+ Up * (Radius * FMath::Sin(AngleRad));
			// Height offset along Forward
			const FVector HeightOffset = Forward * Height;

			// Rotation to follow the spiral
			const FQuat SpiralRotation(Forward, AngleRad);

			FTransform Variant = Context.BaseAttachment;
			Variant.AddToTranslation(RadialOffset + HeightOffset);
			Variant.SetRotation(SpiralRotation * Variant.GetRotation());

			OutVariants.Add(Variant);
		}
	}
};

// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_ConicRange.generated.h"

/**
 * Generator constraint: produces child variants distributed on the surface of a cone.
 * Unlike AngularRange (planar rotation around ONE axis), ConicRange distributes
 * variants in 3D by tilting away from the connector forward and spinning azimuthally.
 */
USTRUCT(BlueprintType, DisplayName="● · Conic Range")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_ConicRange : public FPCGExConstraintGenerator
{
	GENERATED_BODY()

	/** Half-angle of the cone aperture in degrees (0 = no spread, 90 = perpendicular plane) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0, ClampMax = 90))
	float ApertureAngleDegrees = 30.0f;

	/** Tilt the cone axis away from the connector forward direction, around the Right axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = -90, ClampMax = 90))
	float TiltAngleDegrees = 0.0f;

	/** Number of evenly-spaced azimuthal positions on the cone rim */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 1))
	int32 Steps = 4;

	/** Add random azimuthal offset per step for natural variation */
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
		const FVector Center = Context.ParentConnectorWorld.GetTranslation();

		// Tilt: rotate the Right axis itself around Forward so the tilt direction follows azimuth
		// But first, compute the base tilt quaternion (around Right, away from Forward)
		const FQuat Tilt(Right, FMath::DegreesToRadians(ApertureAngleDegrees + TiltAngleDegrees));

		const float AzimuthalStep = 360.0f / static_cast<float>(FMath::Max(1, Steps));

		for (int32 i = 0; i < Steps; ++i)
		{
			float Azimuth = AzimuthalStep * static_cast<float>(i);
			if (bRandomOffset)
			{
				Azimuth += Random.FRand() * AzimuthalStep;
			}

			// Spin around Forward, then tilt
			const FQuat Spin(Forward, FMath::DegreesToRadians(Azimuth));
			const FQuat SpunTiltAxis(Spin.RotateVector(Right), FMath::DegreesToRadians(ApertureAngleDegrees + TiltAngleDegrees));
			const FQuat Combined = SpunTiltAxis * Spin;

			FTransform Variant = Context.BaseAttachment;
			const FVector Offset = Variant.GetTranslation() - Center;
			Variant.SetTranslation(Center + Combined.RotateVector(Offset));
			Variant.SetRotation(Combined * Variant.GetRotation());

			OutVariants.Add(Variant);
		}
	}
};

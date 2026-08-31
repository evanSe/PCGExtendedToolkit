// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_Lattice.generated.h"

/**
 * Generator constraint: places variants on a 2D grid on the connector face plane.
 * Produces CountX * CountY variants with configurable spacing and centering.
 * Useful for wall panels, window arrays, facade grids.
 */
USTRUCT(BlueprintType, DisplayName="● · Lattice")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_Lattice : public FPCGExConstraintGenerator
{
	GENERATED_BODY()

	/** Number of positions along the connector's Right axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 1, ClampMax = 32))
	int32 CountX = 3;

	/** Number of positions along the connector's Up axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 1, ClampMax = 32))
	int32 CountY = 3;

	/** Spacing between positions along the Right axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	float SpacingX = 10.0f;

	/** Spacing between positions along the Up axis */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	float SpacingY = 10.0f;

	/** Center the grid on the connector point */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	bool bCenterGrid = true;

	virtual int32 GetMaxVariants() const override { return CountX * CountY; }

	virtual void GenerateVariants(
		const FPCGExConstraintContext& Context,
		FRandomStream& Random,
		TArray<FTransform>& OutVariants) const override
	{
		const FQuat ConnectorRot = Context.ParentConnectorWorld.GetRotation();
		const FVector Right = ConnectorRot.GetRightVector();
		const FVector Up = ConnectorRot.GetUpVector();

		const float BaseX = bCenterGrid ? -SpacingX * static_cast<float>(CountX - 1) * 0.5f : 0.0f;
		const float BaseY = bCenterGrid ? -SpacingY * static_cast<float>(CountY - 1) * 0.5f : 0.0f;

		for (int32 y = 0; y < CountY; ++y)
		{
			for (int32 x = 0; x < CountX; ++x)
			{
				const FVector GridOffset = Right * (BaseX + static_cast<float>(x) * SpacingX)
					+ Up * (BaseY + static_cast<float>(y) * SpacingY);

				FTransform Variant = Context.BaseAttachment;
				Variant.AddToTranslation(GridOffset);

				OutVariants.Add(Variant);
			}
		}
	}
};

// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"
#include "Growth/Constraints/PCGExConstraint_ContextCondition.h"

#include "PCGExConstraint_ScaleRamp.generated.h"

/**
 * Modifier constraint: scales the placement transform based on a growth context property.
 * Maps a context value (Depth, CumulativeWeight, etc.) from a source range to a scale range.
 * Essential for organic growth where branches thin out with depth.
 */
USTRUCT(BlueprintType, DisplayName="○ · Scale Ramp")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_ScaleRamp : public FPCGExConstraintModifier
{
	GENERATED_BODY()

	/** Which property of the growth context drives the scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	EPCGExContextProperty Source = EPCGExContextProperty::Depth;

	/** Source range minimum — maps to MinScale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	float MinValue = 0.0f;

	/** Source range maximum — maps to MaxScale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	float MaxValue = 5.0f;

	/** Scale factor at MinValue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0.01))
	float MinScale = 1.0f;

	/** Scale factor at MaxValue */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0.01))
	float MaxScale = 0.5f;

	virtual void ApplyModification(
		const FPCGExConstraintContext& Context,
		FTransform& InOutTransform,
		FRandomStream& Random) const override
	{
		double Value = 0.0;

		switch (Source)
		{
		case EPCGExContextProperty::Depth:            Value = static_cast<double>(Context.Depth);              break;
		case EPCGExContextProperty::CumulativeWeight: Value = static_cast<double>(Context.CumulativeWeight);   break;
		case EPCGExContextProperty::ModuleIndex:      Value = static_cast<double>(Context.ChildModuleIndex);   break;
		case EPCGExContextProperty::ConnectorIndex:   Value = static_cast<double>(Context.ChildConnectorIndex); break;
		case EPCGExContextProperty::PlacedCount:      Value = static_cast<double>(Context.PlacedCount);        break;
		}

		const float Range = MaxValue - MinValue;
		const float Alpha = (FMath::Abs(Range) > SMALL_NUMBER)
			? FMath::Clamp(static_cast<float>((Value - MinValue) / Range), 0.0f, 1.0f)
			: 0.0f;

		const float Scale = FMath::Lerp(MinScale, MaxScale, Alpha);
		InOutTransform.SetScale3D(InOutTransform.GetScale3D() * Scale);
	}
};

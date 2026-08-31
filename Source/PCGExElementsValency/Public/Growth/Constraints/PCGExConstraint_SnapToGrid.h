// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Growth/PCGExConnectorConstraintResolver.h"

#include "PCGExConstraint_SnapToGrid.generated.h"

/**
 * Modifier constraint: snaps the attachment point to a grid on the connector face plane.
 * Composable — place after any positional modifier (SurfaceOffset, ArcSurface, etc.)
 * to quantize the result. Grid axes are the connector's Right and Up directions.
 */
USTRUCT(BlueprintType, DisplayName="○ · Snap to Grid")
struct PCGEXELEMENTSVALENCY_API FPCGExConstraint_SnapToGrid : public FPCGExConstraintModifier
{
	GENERATED_BODY()

	/** Grid step size along the connector's Right and Up axes. 0 = no snap on that axis. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint", meta = (ClampMin = 0))
	FVector2D GridStep = FVector2D(10.0, 10.0);

	/** Shift the grid origin on the connector face */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Constraint")
	FVector2D GridOffset = FVector2D::ZeroVector;

	virtual void ApplyModification(
		const FPCGExConstraintContext& Context,
		FTransform& InOutTransform,
		FRandomStream& Random) const override
	{
		const FVector ConnectorCenter = Context.ParentConnectorWorld.GetTranslation();
		const FQuat ConnectorRot = Context.ParentConnectorWorld.GetRotation();
		const FVector Forward = ConnectorRot.GetForwardVector();
		const FVector Right = ConnectorRot.GetRightVector();
		const FVector Up = ConnectorRot.GetUpVector();

		// Decompose current offset from connector center into local axes
		const FVector CurrentOffset = InOutTransform.GetTranslation() - ConnectorCenter;
		float OffsetRight = FVector::DotProduct(CurrentOffset, Right);
		float OffsetUp = FVector::DotProduct(CurrentOffset, Up);
		const float OffsetForward = FVector::DotProduct(CurrentOffset, Forward);

		// Snap to grid
		if (GridStep.X > SMALL_NUMBER)
		{
			OffsetRight = FMath::RoundToFloat((OffsetRight - GridOffset.X) / GridStep.X) * GridStep.X + GridOffset.X;
		}
		if (GridStep.Y > SMALL_NUMBER)
		{
			OffsetUp = FMath::RoundToFloat((OffsetUp - GridOffset.Y) / GridStep.Y) * GridStep.Y + GridOffset.Y;
		}

		// Reconstruct: preserve forward component, snap Right/Up
		InOutTransform.SetTranslation(ConnectorCenter + Forward * OffsetForward + Right * OffsetRight + Up * OffsetUp);
	}
};

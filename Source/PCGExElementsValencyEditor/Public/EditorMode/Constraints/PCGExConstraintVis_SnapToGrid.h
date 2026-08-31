// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "EditorMode/PCGExConstraintVisualizer.h"

/**
 * Visualizer for FPCGExConstraint_SnapToGrid.
 * Draws a grid pattern on the connector face plane.
 */
class PCGEXELEMENTSVALENCYEDITOR_API FSnapToGridVisualizer : public IConstraintVisualizer
{
public:
	virtual void DrawIndicator(
		FPrimitiveDrawInterface* PDI,
		const FTransform& ConnectorWorld,
		const FPCGExConnectorConstraint& Constraint,
		const FLinearColor& Color) const override;

	virtual void DrawZone(
		FPrimitiveDrawInterface* PDI,
		const FTransform& ConnectorWorld,
		const FPCGExConnectorConstraint& Constraint,
		const FLinearColor& Color) const override;

	virtual void DrawDetail(
		FPrimitiveDrawInterface* PDI,
		const FTransform& ConnectorWorld,
		const FPCGExConnectorConstraint& Constraint,
		const FLinearColor& Color,
		bool bIsActiveConstraint) const override;
};

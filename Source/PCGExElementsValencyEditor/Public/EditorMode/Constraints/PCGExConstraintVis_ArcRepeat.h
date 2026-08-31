// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "EditorMode/PCGExConstraintVisualizer.h"

/**
 * Visualizer for FPCGExConstraint_ArcRepeat.
 * Draws evenly-spaced dots along an arc with the arc outline.
 */
class PCGEXELEMENTSVALENCYEDITOR_API FArcRepeatVisualizer : public IConstraintVisualizer
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

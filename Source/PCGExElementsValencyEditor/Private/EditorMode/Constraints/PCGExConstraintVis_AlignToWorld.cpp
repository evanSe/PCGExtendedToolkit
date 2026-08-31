// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_AlignToWorld.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_AlignToWorld.h"

namespace
{
	FVector GetTargetDirection(const FPCGExConstraint_AlignToWorld& Align)
	{
		switch (Align.Direction)
		{
		case EPCGExWorldDirection::WorldUp:      return FVector::UpVector;
		case EPCGExWorldDirection::WorldDown:     return -FVector::UpVector;
		case EPCGExWorldDirection::WorldForward:  return FVector::ForwardVector;
		case EPCGExWorldDirection::WorldRight:    return FVector::RightVector;
		case EPCGExWorldDirection::Custom:        return Align.CustomDirection.GetSafeNormal();
		default:                                  return FVector::UpVector;
		}
	}
}

#pragma region FAlignToWorldVisualizer

void FAlignToWorldVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small up-arrow indicator
	const FVector Center = ConnectorWorld.GetTranslation();
	const auto& Align = static_cast<const FPCGExConstraint_AlignToWorld&>(Constraint);
	const FVector TargetDir = GetTargetDirection(Align);

	const float S = 5.0f;
	PDI->DrawLine(Center, Center + TargetDir * S, Color, SDPG_World, 1.5f);
	PDI->DrawPoint(Center + TargetDir * S, Color, 5.0f, SDPG_World);
}

void FAlignToWorldVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Align = static_cast<const FPCGExConstraint_AlignToWorld&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FVector TargetDir = GetTargetDirection(Align);

	if (TargetDir.IsNearlyZero()) { return; }

	const float ArrowLength = 12.0f;
	const FVector ArrowEnd = Center + TargetDir * ArrowLength;

	// Main arrow line
	PDI->DrawLine(Center, ArrowEnd, Color, SDPG_World, 1.0f);

	// Arrowhead
	FVector Perp1, Perp2;
	TargetDir.FindBestAxisVectors(Perp1, Perp2);
	const float HeadSize = 3.0f;
	PDI->DrawLine(ArrowEnd, ArrowEnd - TargetDir * HeadSize + Perp1 * HeadSize * 0.5f, Color, SDPG_World, 1.0f);
	PDI->DrawLine(ArrowEnd, ArrowEnd - TargetDir * HeadSize - Perp1 * HeadSize * 0.5f, Color, SDPG_World, 1.0f);

	// Show strength as a blended line from current local axis to target
	const FVector LocalAxisDir = PCGExMath::GetDirection(ConnectorWorld.GetRotation(), Align.LocalAxis);
	const FVector BlendedDir = FMath::Lerp(LocalAxisDir, TargetDir, Align.Strength).GetSafeNormal();

	if (!BlendedDir.IsNearlyZero())
	{
		// Dashed line showing current axis direction
		PDI->DrawLine(Center, Center + LocalAxisDir * (ArrowLength * 0.7f), Color * 0.3f, SDPG_World, 0.5f);

		// Solid line showing blended result
		if (Align.Strength < 1.0f - SMALL_NUMBER)
		{
			PDI->DrawLine(Center, Center + BlendedDir * (ArrowLength * 0.8f), Color * 0.6f, SDPG_World, 0.5f);
		}
	}
}

void FAlignToWorldVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const auto& Align = static_cast<const FPCGExConstraint_AlignToWorld&>(Constraint);
	const FVector Center = ConnectorWorld.GetTranslation();
	const FVector TargetDir = GetTargetDirection(Align);

	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;

	// Handle dot at arrow tip
	if (!TargetDir.IsNearlyZero())
	{
		PDI->DrawPoint(Center + TargetDir * 12.0f, HandleColor, 6.0f, SDPG_World);
	}

	// Origin dot
	PDI->DrawPoint(Center, HandleColor, 4.0f, SDPG_World);
}

#pragma endregion

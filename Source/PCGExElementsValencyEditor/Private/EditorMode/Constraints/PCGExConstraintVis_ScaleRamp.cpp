// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_ScaleRamp.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_ScaleRamp.h"

#pragma region FScaleRampVisualizer

void FScaleRampVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small ramp indicator: triangle (small to large)
	const FVector Center = ConnectorWorld.GetTranslation() + ConnectorWorld.GetRotation().GetUpVector() * 5.0f;
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector();
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector();

	PDI->DrawLine(Center - Right * 3.0f, Center + Right * 3.0f, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center + Right * 3.0f, Center - Right * 3.0f + Up * 4.0f, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center - Right * 3.0f + Up * 4.0f, Center - Right * 3.0f, Color, SDPG_World, 1.0f);
}

void FScaleRampVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Ramp = static_cast<const FPCGExConstraint_ScaleRamp&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Forward = Rot.GetForwardVector();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	// Draw scale progression: 5 nested squares going from MinScale to MaxScale
	const float BaseSize = 8.0f;
	const float Spacing = 5.0f;
	const int32 NumSteps = 5;

	for (int32 i = 0; i < NumSteps; ++i)
	{
		const float T = static_cast<float>(i) / static_cast<float>(NumSteps - 1);
		const float Scale = FMath::Lerp(Ramp.MinScale, Ramp.MaxScale, T);
		const float HalfSize = BaseSize * Scale * 0.5f;
		const FVector StepCenter = Center + Forward * (static_cast<float>(i) * Spacing);

		// Draw square at this step
		const FVector TL = StepCenter + Right * (-HalfSize) + Up * HalfSize;
		const FVector TR = StepCenter + Right * HalfSize + Up * HalfSize;
		const FVector BR = StepCenter + Right * HalfSize + Up * (-HalfSize);
		const FVector BL = StepCenter + Right * (-HalfSize) + Up * (-HalfSize);

		const float Intensity = 0.4f + 0.6f * T;
		PDI->DrawLine(TL, TR, Color * Intensity, SDPG_World, 0.5f);
		PDI->DrawLine(TR, BR, Color * Intensity, SDPG_World, 0.5f);
		PDI->DrawLine(BR, BL, Color * Intensity, SDPG_World, 0.5f);
		PDI->DrawLine(BL, TL, Color * Intensity, SDPG_World, 0.5f);
	}

	// Arrow along progression
	const FVector ArrowStart = Center - Forward * 2.0f;
	const FVector ArrowEnd = Center + Forward * (static_cast<float>(NumSteps - 1) * Spacing + 2.0f);
	PDI->DrawLine(ArrowStart, ArrowEnd, Color * 0.4f, SDPG_World, 0.5f);
}

void FScaleRampVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	const FVector Center = ConnectorWorld.GetTranslation();
	PDI->DrawPoint(Center, HandleColor, 5.0f, SDPG_World);
}

#pragma endregion

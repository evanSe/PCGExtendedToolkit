// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_Probability.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_Probability.h"

#pragma region FProbabilityVisualizer

void FProbabilityVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small diamond/dice indicator
	const FVector Center = ConnectorWorld.GetTranslation() + ConnectorWorld.GetRotation().GetUpVector() * 5.0f;
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector() * 3.0f;
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector() * 3.0f;

	PDI->DrawLine(Center - Up, Center + Right, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center + Right, Center + Up, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center + Up, Center - Right, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center - Right, Center - Up, Color, SDPG_World, 1.0f);
}

void FProbabilityVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Prob = static_cast<const FPCGExConstraint_Probability&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	// Draw a "progress bar" showing probability
	const float BarLength = 12.0f;
	const float BarHeight = 3.0f;
	const float FilledLength = BarLength * Prob.Chance;

	const FVector BarStart = Center + Up * 8.0f - Right * (BarLength * 0.5f);

	// Bar outline
	const FVector BL = BarStart;
	const FVector BR = BarStart + Right * BarLength;
	const FVector TL = BarStart + Up * BarHeight;
	const FVector TR = BarStart + Right * BarLength + Up * BarHeight;

	PDI->DrawLine(BL, BR, Color * 0.5f, SDPG_World, 0.5f);
	PDI->DrawLine(BR, TR, Color * 0.5f, SDPG_World, 0.5f);
	PDI->DrawLine(TR, TL, Color * 0.5f, SDPG_World, 0.5f);
	PDI->DrawLine(TL, BL, Color * 0.5f, SDPG_World, 0.5f);

	// Filled portion
	if (FilledLength > SMALL_NUMBER)
	{
		const FVector FillBR = BarStart + Right * FilledLength;
		const FVector FillTR = BarStart + Right * FilledLength + Up * BarHeight;
		PDI->DrawLine(BL, FillBR, Color, SDPG_World, 1.5f);
		PDI->DrawLine(FillBR, FillTR, Color, SDPG_World, 1.5f);
		PDI->DrawLine(FillTR, TL, Color, SDPG_World, 1.5f);
	}
}

void FProbabilityVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const auto& Prob = static_cast<const FPCGExConstraint_Probability&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	// Handle dot at the probability threshold position
	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	const float BarLength = 12.0f;
	const float BarHeight = 3.0f;
	const FVector BarStart = Center + Up * 8.0f - Right * (BarLength * 0.5f);
	const FVector ThresholdPt = BarStart + Right * (BarLength * Prob.Chance) + Up * (BarHeight * 0.5f);
	PDI->DrawPoint(ThresholdPt, HandleColor, 6.0f, SDPG_World);
}

#pragma endregion

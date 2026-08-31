// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_SnapToGrid.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_SnapToGrid.h"

#pragma region FSnapToGridVisualizer

void FSnapToGridVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small grid indicator: hash pattern
	const FVector Center = ConnectorWorld.GetTranslation() + ConnectorWorld.GetRotation().GetUpVector() * 5.0f;
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector() * 2.5f;
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector() * 2.5f;

	// Draw small 2x2 grid
	for (int32 i = 0; i <= 2; ++i)
	{
		const float T = static_cast<float>(i) / 2.0f - 0.5f;
		PDI->DrawLine(Center + Right * T * 2.0f - Up, Center + Right * T * 2.0f + Up, Color * 0.8f, SDPG_World, 0.5f);
		PDI->DrawLine(Center - Right + Up * T * 2.0f, Center + Right + Up * T * 2.0f, Color * 0.8f, SDPG_World, 0.5f);
	}
}

void FSnapToGridVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Grid = static_cast<const FPCGExConstraint_SnapToGrid&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	// Draw a 5x5 grid centered on the connector
	const float StepX = FMath::Max(Grid.GridStep.X, 1.0f);
	const float StepY = FMath::Max(Grid.GridStep.Y, 1.0f);
	const int32 Lines = 2; // lines on each side of center

	const float ExtentX = StepX * static_cast<float>(Lines);
	const float ExtentY = StepY * static_cast<float>(Lines);

	// Vertical lines (along Up)
	for (int32 i = -Lines; i <= Lines; ++i)
	{
		const float X = static_cast<float>(i) * StepX + Grid.GridOffset.X;
		const FVector Base = Center + Right * X;
		PDI->DrawLine(Base - Up * ExtentY, Base + Up * ExtentY, Color * 0.5f, SDPG_World, 0.5f);
	}

	// Horizontal lines (along Right)
	for (int32 i = -Lines; i <= Lines; ++i)
	{
		const float Y = static_cast<float>(i) * StepY + Grid.GridOffset.Y;
		const FVector Base = Center + Up * Y;
		PDI->DrawLine(Base - Right * ExtentX, Base + Right * ExtentX, Color * 0.5f, SDPG_World, 0.5f);
	}

	// Intersection dots
	for (int32 iy = -Lines; iy <= Lines; ++iy)
	{
		for (int32 ix = -Lines; ix <= Lines; ++ix)
		{
			const FVector Pt = Center
				+ Right * (static_cast<float>(ix) * StepX + Grid.GridOffset.X)
				+ Up * (static_cast<float>(iy) * StepY + Grid.GridOffset.Y);
			PDI->DrawPoint(Pt, Color * 0.7f, 3.0f, SDPG_World);
		}
	}
}

void FSnapToGridVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	// Highlight center dot
	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	const FVector Center = ConnectorWorld.GetTranslation();
	PDI->DrawPoint(Center, HandleColor, 6.0f, SDPG_World);
}

#pragma endregion

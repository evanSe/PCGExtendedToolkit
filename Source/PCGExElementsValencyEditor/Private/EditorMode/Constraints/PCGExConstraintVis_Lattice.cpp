// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_Lattice.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_Lattice.h"

#pragma region FLatticeVisualizer

void FLatticeVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small 2x2 grid of dots
	const FVector Center = ConnectorWorld.GetTranslation() + ConnectorWorld.GetRotation().GetUpVector() * 5.0f;
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector() * 2.0f;
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector() * 2.0f;

	for (int32 y = 0; y < 2; ++y)
	{
		for (int32 x = 0; x < 2; ++x)
		{
			const FVector Pt = Center + Right * (static_cast<float>(x) - 0.5f) + Up * (static_cast<float>(y) - 0.5f);
			PDI->DrawPoint(Pt, Color, 3.0f, SDPG_World);
		}
	}
}

void FLatticeVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Grid = static_cast<const FPCGExConstraint_Lattice&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float BaseX = Grid.bCenterGrid ? -Grid.SpacingX * static_cast<float>(Grid.CountX - 1) * 0.5f : 0.0f;
	const float BaseY = Grid.bCenterGrid ? -Grid.SpacingY * static_cast<float>(Grid.CountY - 1) * 0.5f : 0.0f;

	// Draw grid outline
	const FVector TL = Center + Right * BaseX + Up * (BaseY + Grid.SpacingY * static_cast<float>(Grid.CountY - 1));
	const FVector TR = Center + Right * (BaseX + Grid.SpacingX * static_cast<float>(Grid.CountX - 1)) + Up * (BaseY + Grid.SpacingY * static_cast<float>(Grid.CountY - 1));
	const FVector BR = Center + Right * (BaseX + Grid.SpacingX * static_cast<float>(Grid.CountX - 1)) + Up * BaseY;
	const FVector BL = Center + Right * BaseX + Up * BaseY;

	PDI->DrawLine(TL, TR, Color * 0.5f, SDPG_World, 0.5f);
	PDI->DrawLine(TR, BR, Color * 0.5f, SDPG_World, 0.5f);
	PDI->DrawLine(BR, BL, Color * 0.5f, SDPG_World, 0.5f);
	PDI->DrawLine(BL, TL, Color * 0.5f, SDPG_World, 0.5f);

	// Draw grid points
	for (int32 y = 0; y < Grid.CountY; ++y)
	{
		for (int32 x = 0; x < Grid.CountX; ++x)
		{
			const FVector Pt = Center
				+ Right * (BaseX + static_cast<float>(x) * Grid.SpacingX)
				+ Up * (BaseY + static_cast<float>(y) * Grid.SpacingY);
			PDI->DrawPoint(Pt, Color, 4.0f, SDPG_World);
		}
	}
}

void FLatticeVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const auto& Grid = static_cast<const FPCGExConstraint_Lattice&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float BaseX = Grid.bCenterGrid ? -Grid.SpacingX * static_cast<float>(Grid.CountX - 1) * 0.5f : 0.0f;
	const float BaseY = Grid.bCenterGrid ? -Grid.SpacingY * static_cast<float>(Grid.CountY - 1) * 0.5f : 0.0f;

	// Draw internal grid lines
	const FLinearColor LineColor = (bIsActiveConstraint ? Color : Color * 0.8f) * 0.4f;
	for (int32 x = 0; x < Grid.CountX; ++x)
	{
		const float PosX = BaseX + static_cast<float>(x) * Grid.SpacingX;
		const FVector Bottom = Center + Right * PosX + Up * BaseY;
		const FVector Top = Center + Right * PosX + Up * (BaseY + Grid.SpacingY * static_cast<float>(Grid.CountY - 1));
		PDI->DrawLine(Bottom, Top, LineColor, SDPG_World, 0.5f);
	}
	for (int32 y = 0; y < Grid.CountY; ++y)
	{
		const float PosY = BaseY + static_cast<float>(y) * Grid.SpacingY;
		const FVector Left = Center + Right * BaseX + Up * PosY;
		const FVector RightEnd = Center + Right * (BaseX + Grid.SpacingX * static_cast<float>(Grid.CountX - 1)) + Up * PosY;
		PDI->DrawLine(Left, RightEnd, LineColor, SDPG_World, 0.5f);
	}

	// Center cross
	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	PDI->DrawPoint(Center, HandleColor, 6.0f, SDPG_World);
}

#pragma endregion

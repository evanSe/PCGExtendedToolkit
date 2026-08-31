// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_ArcRepeat.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_ArcRepeat.h"

#pragma region FArcRepeatVisualizer

void FArcRepeatVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small dashed arc indicator
	const FVector Center = ConnectorWorld.GetTranslation();
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector();
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector();

	const float R = 4.0f;
	for (int32 i = 0; i < 4; ++i)
	{
		const float Angle = FMath::DegreesToRadians(static_cast<float>(i) * 90.0f);
		const FVector Pt = Center + Up * 5.0f + Right * (R * FMath::Cos(Angle)) + Up * (R * FMath::Sin(Angle));
		PDI->DrawPoint(Pt, Color, 3.0f, SDPG_World);
	}
}

void FArcRepeatVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& ArcR = static_cast<const FPCGExConstraint_ArcRepeat&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float MinAngle = ArcR.GetMinAngle();
	const float MaxAngle = ArcR.GetMaxAngle();
	const int32 ArcSegments = FMath::Max(16, ArcR.Steps * 4);

	// Draw arc outline
	const float MinRad = FMath::DegreesToRadians(MinAngle);
	const float MaxRad = FMath::DegreesToRadians(MaxAngle);
	const float Range = MaxRad - MinRad;

	FVector Prev = Center + (Right * FMath::Cos(MinRad) + Up * FMath::Sin(MinRad)) * ArcR.Radius;
	for (int32 i = 1; i <= ArcSegments; ++i)
	{
		const float T = static_cast<float>(i) / static_cast<float>(ArcSegments);
		const float Angle = MinRad + Range * T;
		const FVector Point = Center + (Right * FMath::Cos(Angle) + Up * FMath::Sin(Angle)) * ArcR.Radius;
		PDI->DrawLine(Prev, Point, Color * 0.5f, SDPG_World, 0.5f);
		Prev = Point;
	}

	// Draw step position dots on the arc
	const float StepRange = ArcR.HalfWidthDegrees * 2.0f;
	const float StepSize = (ArcR.Steps > 1) ? (StepRange / static_cast<float>(ArcR.Steps)) : 0.0f;

	for (int32 i = 0; i < ArcR.Steps; ++i)
	{
		const float Angle = FMath::DegreesToRadians(MinAngle + StepSize * static_cast<float>(i));
		const FVector StepPoint = Center + (Right * FMath::Cos(Angle) + Up * FMath::Sin(Angle)) * ArcR.Radius;
		PDI->DrawPoint(StepPoint, Color, 5.0f, SDPG_World);
	}

	// Radial lines at bounds
	const FVector MinDir = Right * FMath::Cos(MinRad) + Up * FMath::Sin(MinRad);
	const FVector MaxDir = Right * FMath::Cos(MaxRad) + Up * FMath::Sin(MaxRad);
	PDI->DrawLine(Center, Center + MinDir * ArcR.Radius, Color * 0.4f, SDPG_World, 0.5f);
	PDI->DrawLine(Center, Center + MaxDir * ArcR.Radius, Color * 0.4f, SDPG_World, 0.5f);
}

void FArcRepeatVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const auto& ArcR = static_cast<const FPCGExConstraint_ArcRepeat&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	// Radius handle at center angle
	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	const float CenterRad = FMath::DegreesToRadians(ArcR.CenterAngleDegrees);
	const FVector CenterDir = Right * FMath::Cos(CenterRad) + Up * FMath::Sin(CenterRad);
	PDI->DrawLine(Center, Center + CenterDir * ArcR.Radius, HandleColor * 0.5f, SDPG_World, 0.5f);
	PDI->DrawPoint(Center + CenterDir * ArcR.Radius, HandleColor, 6.0f, SDPG_World);
}

#pragma endregion

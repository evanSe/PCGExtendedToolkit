// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_Spiral.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_Spiral.h"

#pragma region FSpiralVisualizer

void FSpiralVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small spiral indicator
	const FVector Center = ConnectorWorld.GetTranslation();
	const FVector Forward = ConnectorWorld.GetRotation().GetForwardVector();
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector();
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector();

	const float R = 3.0f;
	const int32 Segments = 8;
	FVector Prev = Center + Right * R;

	for (int32 i = 1; i <= Segments; ++i)
	{
		const float T = static_cast<float>(i) / static_cast<float>(Segments);
		const float Angle = T * PI * 1.5f;
		const FVector Pt = Center
			+ Right * (R * FMath::Cos(Angle))
			+ Up * (R * FMath::Sin(Angle))
			+ Forward * (T * 4.0f);
		PDI->DrawLine(Prev, Pt, Color, SDPG_World, 1.0f);
		Prev = Pt;
	}
}

void FSpiralVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Spiral = static_cast<const FPCGExConstraint_Spiral&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Forward = Rot.GetForwardVector();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float HeightPerDegree = (FMath::Abs(360.0f) > SMALL_NUMBER) ? Spiral.HeightPerRevolution / 360.0f : 0.0f;
	const int32 CurveSegments = FMath::Max(32, Spiral.Steps * 8);
	const float TotalAngleDeg = Spiral.TotalRotationDegrees;

	// Draw helix curve
	FVector Prev = Center + Right * Spiral.Radius;
	for (int32 i = 1; i <= CurveSegments; ++i)
	{
		const float T = static_cast<float>(i) / static_cast<float>(CurveSegments);
		const float AngleDeg = TotalAngleDeg * T;
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		const float Height = AngleDeg * HeightPerDegree;

		const FVector Pt = Center
			+ Right * (Spiral.Radius * FMath::Cos(AngleRad))
			+ Up * (Spiral.Radius * FMath::Sin(AngleRad))
			+ Forward * Height;
		PDI->DrawLine(Prev, Pt, Color * 0.6f, SDPG_World, 0.5f);
		Prev = Pt;
	}

	// Draw step position dots
	const float StepAngle = (Spiral.Steps > 1)
		? TotalAngleDeg / static_cast<float>(Spiral.Steps - 1)
		: 0.0f;

	for (int32 i = 0; i < Spiral.Steps; ++i)
	{
		const float AngleDeg = StepAngle * static_cast<float>(i);
		const float AngleRad = FMath::DegreesToRadians(AngleDeg);
		const float Height = AngleDeg * HeightPerDegree;

		const FVector StepPt = Center
			+ Right * (Spiral.Radius * FMath::Cos(AngleRad))
			+ Up * (Spiral.Radius * FMath::Sin(AngleRad))
			+ Forward * Height;
		PDI->DrawPoint(StepPt, Color, 5.0f, SDPG_World);

		// Radial line from axis to step
		const FVector AxisPt = Center + Forward * Height;
		PDI->DrawLine(AxisPt, StepPt, Color * 0.3f, SDPG_World, 0.5f);
	}

	// Draw central axis
	const float TotalHeight = TotalAngleDeg * HeightPerDegree;
	PDI->DrawLine(Center, Center + Forward * TotalHeight, Color * 0.25f, SDPG_World, 0.5f);
}

void FSpiralVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	const FVector Center = ConnectorWorld.GetTranslation();

	// Start/end dots
	PDI->DrawPoint(Center, HandleColor, 6.0f, SDPG_World);

	const auto& Spiral = static_cast<const FPCGExConstraint_Spiral&>(Constraint);
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Forward = Rot.GetForwardVector();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float HeightPerDegree = (FMath::Abs(360.0f) > SMALL_NUMBER) ? Spiral.HeightPerRevolution / 360.0f : 0.0f;
	const float EndAngleRad = FMath::DegreesToRadians(Spiral.TotalRotationDegrees);
	const float EndHeight = Spiral.TotalRotationDegrees * HeightPerDegree;

	const FVector EndPt = Center
		+ Right * (Spiral.Radius * FMath::Cos(EndAngleRad))
		+ Up * (Spiral.Radius * FMath::Sin(EndAngleRad))
		+ Forward * EndHeight;
	PDI->DrawPoint(EndPt, HandleColor, 6.0f, SDPG_World);
}

#pragma endregion

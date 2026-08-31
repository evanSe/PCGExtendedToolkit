// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_ConicRange.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_ConicRange.h"

#pragma region FConicRangeVisualizer

void FConicRangeVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small cone indicator: V-shape pointing forward
	const FVector Center = ConnectorWorld.GetTranslation();
	const FVector Forward = ConnectorWorld.GetRotation().GetForwardVector();
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector();

	const float S = 5.0f;
	PDI->DrawLine(Center, Center + Forward * S + Right * S * 0.5f, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center, Center + Forward * S - Right * S * 0.5f, Color, SDPG_World, 1.0f);
	PDI->DrawLine(Center + Forward * S + Right * S * 0.5f, Center + Forward * S - Right * S * 0.5f, Color, SDPG_World, 1.0f);
}

void FConicRangeVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Conic = static_cast<const FPCGExConstraint_ConicRange&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Forward = Rot.GetForwardVector();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float ConeLength = 15.0f;
	const float ApertureRad = FMath::DegreesToRadians(Conic.ApertureAngleDegrees + Conic.TiltAngleDegrees);
	const float RimRadius = ConeLength * FMath::Tan(FMath::Clamp(ApertureRad, 0.0f, PI * 0.48f));

	// Cone tip is at center, rim is at Center + Forward * ConeLength
	const FVector RimCenter = Center + Forward * ConeLength;

	// Draw rim circle
	const int32 RimSegments = 24;
	FVector PrevRimPoint = RimCenter + Right * RimRadius;
	for (int32 i = 1; i <= RimSegments; ++i)
	{
		const float Angle = 2.0f * PI * static_cast<float>(i) / static_cast<float>(RimSegments);
		const FVector RimPoint = RimCenter + Right * (RimRadius * FMath::Cos(Angle)) + Up * (RimRadius * FMath::Sin(Angle));
		PDI->DrawLine(PrevRimPoint, RimPoint, Color, SDPG_World, 0.5f);
		PrevRimPoint = RimPoint;
	}

	// Draw 4 lines from tip to rim
	for (int32 i = 0; i < 4; ++i)
	{
		const float Angle = PI * 0.5f * static_cast<float>(i);
		const FVector RimPoint = RimCenter + Right * (RimRadius * FMath::Cos(Angle)) + Up * (RimRadius * FMath::Sin(Angle));
		PDI->DrawLine(Center, RimPoint, Color * 0.6f, SDPG_World, 0.5f);
	}

	// Draw axis indicator
	PDI->DrawLine(Center, Center + Forward * (ConeLength + 4.0f), Color * 0.3f, SDPG_World, 0.5f);
}

void FConicRangeVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const auto& Conic = static_cast<const FPCGExConstraint_ConicRange&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Forward = Rot.GetForwardVector();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float ConeLength = 15.0f;
	const float ApertureRad = FMath::DegreesToRadians(Conic.ApertureAngleDegrees + Conic.TiltAngleDegrees);
	const float RimRadius = ConeLength * FMath::Tan(FMath::Clamp(ApertureRad, 0.0f, PI * 0.48f));
	const FVector RimCenter = Center + Forward * ConeLength;

	// Draw step position dots on rim
	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;
	const float AzimuthalStep = 360.0f / static_cast<float>(FMath::Max(1, Conic.Steps));
	for (int32 i = 0; i < Conic.Steps; ++i)
	{
		const float Angle = FMath::DegreesToRadians(AzimuthalStep * static_cast<float>(i));
		const FVector StepPoint = RimCenter + Right * (RimRadius * FMath::Cos(Angle)) + Up * (RimRadius * FMath::Sin(Angle));
		PDI->DrawPoint(StepPoint, HandleColor, 5.0f, SDPG_World);
	}

	// Tip dot
	PDI->DrawPoint(Center, HandleColor, 4.0f, SDPG_World);
}

#pragma endregion

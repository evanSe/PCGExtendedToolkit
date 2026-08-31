// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "EditorMode/Constraints/PCGExConstraintVis_ArcSurface.h"

#include "SceneManagement.h"
#include "Growth/Constraints/PCGExConstraint_ArcSurface.h"

namespace
{
	void DrawArcOnFace(FPrimitiveDrawInterface* PDI, const FVector& Center,
		const FVector& PlaneRight, const FVector& PlaneUp,
		float Radius, float MinAngleDeg, float MaxAngleDeg,
		int32 Segments, const FLinearColor& Color, float Thickness)
	{
		if (Radius < SMALL_NUMBER) { return; }
		const float MinRad = FMath::DegreesToRadians(MinAngleDeg);
		const float MaxRad = FMath::DegreesToRadians(MaxAngleDeg);
		const float Range = MaxRad - MinRad;

		FVector Prev = Center + (PlaneRight * FMath::Cos(MinRad) + PlaneUp * FMath::Sin(MinRad)) * Radius;
		for (int32 i = 1; i <= Segments; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(Segments);
			const float Angle = MinRad + Range * T;
			const FVector Point = Center + (PlaneRight * FMath::Cos(Angle) + PlaneUp * FMath::Sin(Angle)) * Radius;
			PDI->DrawLine(Prev, Point, Color, SDPG_World, Thickness);
			Prev = Point;
		}
	}
}

#pragma region FArcSurfaceVisualizer

void FArcSurfaceVisualizer::DrawIndicator(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	// Small arc indicator
	const FVector Center = ConnectorWorld.GetTranslation();
	const FVector Right = ConnectorWorld.GetRotation().GetRightVector();
	const FVector Up = ConnectorWorld.GetRotation().GetUpVector();

	DrawArcOnFace(PDI, Center + Up * 5.0f, Right, Up, 3.0f, -90.0f, 90.0f, 6, Color, 1.0f);
}

void FArcSurfaceVisualizer::DrawZone(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color) const
{
	const auto& Arc = static_cast<const FPCGExConstraint_ArcSurface&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const float MinAngle = Arc.GetMinAngle();
	const float MaxAngle = Arc.GetMaxAngle();
	const int32 ArcSegments = FMath::Max(12, static_cast<int32>((MaxAngle - MinAngle) / 5.0f));

	// Draw outer arc
	DrawArcOnFace(PDI, Center, Right, Up, Arc.OuterRadius, MinAngle, MaxAngle, ArcSegments, Color, 1.0f);

	// Draw inner arc (if inner radius > 0)
	if (Arc.InnerRadius > SMALL_NUMBER)
	{
		DrawArcOnFace(PDI, Center, Right, Up, Arc.InnerRadius, MinAngle, MaxAngle, ArcSegments, Color * 0.7f, 0.5f);
	}

	// Draw radial lines at min and max angles
	const float MinRad = FMath::DegreesToRadians(MinAngle);
	const float MaxRad = FMath::DegreesToRadians(MaxAngle);

	const FVector MinDirInner = (Right * FMath::Cos(MinRad) + Up * FMath::Sin(MinRad));
	const FVector MaxDirInner = (Right * FMath::Cos(MaxRad) + Up * FMath::Sin(MaxRad));

	const float InnerR = FMath::Max(Arc.InnerRadius, 0.0f);
	PDI->DrawLine(Center + MinDirInner * InnerR, Center + MinDirInner * Arc.OuterRadius, Color * 0.6f, SDPG_World, 0.5f);
	PDI->DrawLine(Center + MaxDirInner * InnerR, Center + MaxDirInner * Arc.OuterRadius, Color * 0.6f, SDPG_World, 0.5f);

	// Cross-hair at center
	PDI->DrawLine(Center + Right * (-2.0f), Center + Right * 2.0f, Color * 0.4f, SDPG_World, 0.5f);
	PDI->DrawLine(Center + Up * (-2.0f), Center + Up * 2.0f, Color * 0.4f, SDPG_World, 0.5f);
}

void FArcSurfaceVisualizer::DrawDetail(
	FPrimitiveDrawInterface* PDI,
	const FTransform& ConnectorWorld,
	const FPCGExConnectorConstraint& Constraint,
	const FLinearColor& Color,
	bool bIsActiveConstraint) const
{
	DrawZone(PDI, ConnectorWorld, Constraint, Color);

	const auto& Arc = static_cast<const FPCGExConstraint_ArcSurface&>(Constraint);

	const FVector Center = ConnectorWorld.GetTranslation();
	const FQuat Rot = ConnectorWorld.GetRotation();
	const FVector Right = Rot.GetRightVector();
	const FVector Up = Rot.GetUpVector();

	const FLinearColor HandleColor = bIsActiveConstraint ? Color : Color * 0.8f;

	// Handle dots at arc endpoints (outer radius)
	const float MinRad = FMath::DegreesToRadians(Arc.GetMinAngle());
	const float MaxRad = FMath::DegreesToRadians(Arc.GetMaxAngle());
	PDI->DrawPoint(Center + (Right * FMath::Cos(MinRad) + Up * FMath::Sin(MinRad)) * Arc.OuterRadius, HandleColor, 5.0f, SDPG_World);
	PDI->DrawPoint(Center + (Right * FMath::Cos(MaxRad) + Up * FMath::Sin(MaxRad)) * Arc.OuterRadius, HandleColor, 5.0f, SDPG_World);

	// Center angle indicator
	const float CenterRad = FMath::DegreesToRadians(Arc.CenterAngleDegrees);
	const FVector CenterDir = Right * FMath::Cos(CenterRad) + Up * FMath::Sin(CenterRad);
	PDI->DrawLine(Center, Center + CenterDir * Arc.OuterRadius, HandleColor * 0.5f, SDPG_World, 0.5f);
	PDI->DrawPoint(Center + CenterDir * (Arc.InnerRadius + Arc.OuterRadius) * 0.5f, HandleColor, 4.0f, SDPG_World);
}

#pragma endregion

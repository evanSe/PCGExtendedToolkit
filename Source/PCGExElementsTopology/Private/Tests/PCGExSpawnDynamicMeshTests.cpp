// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Elements/PCGExSpawnDynamicMesh.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/CapsuleComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "Data/PCGDynamicMeshData.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Misc/AutomationTest.h"
#include "PhysicsEngine/BodySetup.h"
#include "UDynamicMesh.h"

namespace PCGExSpawnDynamicMeshTests
{
	struct FScopedCollisionWorld
	{
		UWorld* World = nullptr;

		FScopedCollisionWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, false, TEXT("PCGExSpawnDynamicMeshCollisionWorld"));
			if (GEngine && World)
			{
				FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
				WorldContext.SetCurrentWorld(World);
			}
		}

		~FScopedCollisionWorld()
		{
			if (!World)
			{
				return;
			}

			if (GEngine)
			{
				GEngine->DestroyWorldContext(World);
			}

			World->DestroyWorld(false);
			World = nullptr;
		}
	};

	UE::Geometry::FDynamicMesh3 MakeCollisionSlab()
	{
		UE::Geometry::FDynamicMesh3 Mesh;
		const int32 Top00 = Mesh.AppendVertex(FVector3d(-200.0, -200.0, 0.0));
		const int32 Top10 = Mesh.AppendVertex(FVector3d(200.0, -200.0, 0.0));
		const int32 Top11 = Mesh.AppendVertex(FVector3d(200.0, 200.0, 0.0));
		const int32 Top01 = Mesh.AppendVertex(FVector3d(-200.0, 200.0, 0.0));
		const int32 Bottom00 = Mesh.AppendVertex(FVector3d(-200.0, -200.0, -10.0));
		const int32 Bottom10 = Mesh.AppendVertex(FVector3d(200.0, -200.0, -10.0));
		const int32 Bottom11 = Mesh.AppendVertex(FVector3d(200.0, 200.0, -10.0));
		const int32 Bottom01 = Mesh.AppendVertex(FVector3d(-200.0, 200.0, -10.0));

		Mesh.AppendTriangle(Top00, Top10, Top11);
		Mesh.AppendTriangle(Top00, Top11, Top01);
		Mesh.AppendTriangle(Bottom00, Bottom11, Bottom10);
		Mesh.AppendTriangle(Bottom00, Bottom01, Bottom11);
		Mesh.AppendTriangle(Top00, Bottom10, Top10);
		Mesh.AppendTriangle(Top00, Bottom00, Bottom10);
		Mesh.AppendTriangle(Top10, Bottom11, Top11);
		Mesh.AppendTriangle(Top10, Bottom10, Bottom11);
		Mesh.AppendTriangle(Top11, Bottom01, Top01);
		Mesh.AppendTriangle(Top11, Bottom11, Bottom01);
		Mesh.AppendTriangle(Top01, Bottom00, Top00);
		Mesh.AppendTriangle(Top01, Bottom01, Bottom00);
		return Mesh;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPCGExSpawnDynamicMeshMaterialHandoffTest,
	"PCGEx.Topology.SpawnDynamicMesh.MaterialHandoff",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPCGExSpawnDynamicMeshMaterialHandoffTest::RunTest(const FString& Parameters)
{
	UE::Geometry::FDynamicMesh3 Mesh;
	const int32 A = Mesh.AppendVertex(FVector3d(0.0, 0.0, 0.0));
	const int32 B = Mesh.AppendVertex(FVector3d(100.0, 0.0, 0.0));
	const int32 C = Mesh.AppendVertex(FVector3d(0.0, 100.0, 0.0));
	Mesh.AppendTriangle(A, B, C);

	UPCGDynamicMeshData* MeshData = NewObject<UPCGDynamicMeshData>();
	MeshData->Initialize(MoveTemp(Mesh));
	UMaterialInterface* Material = UMaterial::GetDefaultMaterial(MD_Surface);
	const TArray<UMaterialInterface*> Materials {Material};
	MeshData->SetMaterials(Materials);

	UDynamicMeshComponent* Component = NewObject<UDynamicMeshComponent>();
	PCGExSpawnDynamicMesh::InitializeComponentFromData(*Component, *MeshData);

	TestEqual(TEXT("Geometry reaches the spawned component"), Component->GetDynamicMesh()->GetTriangleCount(), 1);
	TestEqual(TEXT("Authored material reaches the spawned component"), Component->GetMaterial(0), Material);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPCGExSpawnDynamicMeshCollisionFinalizationTest,
	"PCGEx.Topology.SpawnDynamicMesh.CollisionFinalization",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPCGExSpawnDynamicMeshCollisionFinalizationTest::RunTest(const FString& Parameters)
{
	using namespace PCGExSpawnDynamicMeshTests;

	FScopedCollisionWorld TestWorld;
	TestNotNull(TEXT("Transient collision world was created"), TestWorld.World);
	if (!TestWorld.World)
	{
		return false;
	}

	AActor* MeshActor = TestWorld.World->SpawnActor<AActor>();
	TestNotNull(TEXT("Dynamic mesh owner was spawned"), MeshActor);
	if (!MeshActor)
	{
		return false;
	}

	UDynamicMeshComponent* Component = NewObject<UDynamicMeshComponent>(MeshActor, TEXT("GeneratedRoadMesh"));
	MeshActor->SetRootComponent(Component);
	MeshActor->AddInstanceComponent(Component);

	UPCGDynamicMeshData* MeshData = NewObject<UPCGDynamicMeshData>();
	MeshData->Initialize(MakeCollisionSlab());
	PCGExSpawnDynamicMesh::InitializeComponentFromData(*Component, *MeshData);

	FPCGExDynamicMeshDescriptor Descriptor;
	Descriptor.BodyInstance.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Descriptor.BodyInstance.SetObjectType(ECC_WorldStatic);
	Descriptor.BodyInstance.SetResponseToAllChannels(ECR_Block);
	Descriptor.bUseAsyncCooking = false;
	Descriptor.bEnableComplexCollision = true;
	Descriptor.bDeferCollisionUpdates = false;
	Descriptor.InitComponent(Component);

	TestEqual(
		TEXT("Component requests the generated mesh as exact collision"),
		Component->CollisionType.GetValue(),
		ECollisionTraceFlag::CTF_UseComplexAsSimple);
	TestNotEqual(
		TEXT("Pre-finalization BodySetup still contains the early default collision cook"),
		Component->GetBodySetup()->CollisionTraceFlag.GetValue(),
		ECollisionTraceFlag::CTF_UseComplexAsSimple);

	PCGExSpawnDynamicMesh::FinalizeComponentCollision(*Component);

	TestEqual(
		TEXT("Finalized BodySetup uses the generated triangles as simple collision"),
		Component->GetBodySetup()->CollisionTraceFlag.GetValue(),
		ECollisionTraceFlag::CTF_UseComplexAsSimple);
	TestTrue(
		TEXT("Finalized component exposes triangle collision data"),
		Component->ContainsPhysicsTriMeshData(false));

	Component->RegisterComponent();

	UCapsuleComponent* MoverCapsule = NewObject<UCapsuleComponent>();
	MoverCapsule->InitCapsuleSize(34.0f, 88.0f);
	MoverCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MoverCapsule->SetCollisionObjectType(ECC_Pawn);
	MoverCapsule->SetCollisionResponseToAllChannels(ECR_Block);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(PCGExSpawnDynamicMeshMoverSweep), false);
	FCollisionResponseParams ResponseParams;
	MoverCapsule->InitSweepCollisionParams(QueryParams, ResponseParams);

	FHitResult Hit;
	const bool bHit = TestWorld.World->SweepSingleByChannel(
		Hit,
		FVector(0.0, 0.0, 200.0),
		FVector(0.0, 0.0, -200.0),
		FQuat::Identity,
		MoverCapsule->GetCollisionObjectType(),
		MoverCapsule->GetCollisionShape(),
		QueryParams,
		ResponseParams);

	TestTrue(TEXT("Mover-style capsule sweep blocks on the exact generated mesh"), bHit);
	TestEqual(TEXT("Capsule sweep hit the generated component"), Hit.GetComponent(), static_cast<UPrimitiveComponent*>(Component));
	TestTrue(TEXT("Generated road surface is walkable"), Hit.ImpactNormal.Dot(FVector::UpVector) > 0.99f);
	return true;
}
#endif

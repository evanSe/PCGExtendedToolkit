// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Elements/PCGExSpawnDynamicMesh.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Components/DynamicMeshComponent.h"
#include "Data/PCGDynamicMeshData.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "MaterialDomain.h"
#include "Materials/Material.h"
#include "Misc/AutomationTest.h"
#include "UDynamicMesh.h"

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
#endif

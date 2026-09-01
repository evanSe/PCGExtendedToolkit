// Copyright 2026 Timothe Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Misc/AutomationTest.h"
#include "Sketch/PCGExClusterSketchModel.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FPCGExClusterSketchDetachBranchTest,
	"PCGEx.ClusterSketch.Authoring.DetachBranchPreservesTopology",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPCGExClusterSketchDetachBranchTest::RunTest(const FString& Parameters)
{
	FPCGExClusterSketchModel Model;
	const int32 Junction = Model.AddVertex(FTransform(FVector(0.0, 0.0, 0.0)), 17);
	const int32 MainA = Model.AddVertex(FTransform(FVector(-100.0, 0.0, 0.0)));
	const int32 MainB = Model.AddVertex(FTransform(FVector(100.0, 0.0, 0.0)));
	const int32 BranchA = Model.AddVertex(FTransform(FVector(0.0, 100.0, 0.0)));
	const int32 BranchB = Model.AddVertex(FTransform(FVector(0.0, 200.0, 0.0)));

	Model.Connect(MainA, Junction);
	Model.Connect(Junction, MainB);
	const int32 BranchEdge = Model.Connect(Junction, BranchA);
	Model.Connect(BranchA, BranchB);
	Model.Edges[BranchEdge].DataId = 33;
	const uint32 BranchEdgeId = Model.Edges[BranchEdge].Id;

	const int32 Detached = Model.DetachEdgeEndpoint(BranchEdge, Junction);
	TestEqual(TEXT("One endpoint is duplicated"), Model.NumVertices(), 6);
	TestEqual(TEXT("No edge is deleted"), Model.NumEdges(), 4);
	TestTrue(TEXT("Detached endpoint is returned"), Model.Vertices.IsValidIndex(Detached));
	TestEqual(TEXT("Vertex data is inherited"), Model.Vertices[Detached].DataId, static_cast<uint32>(17));
	TestEqual(TEXT("Branch edge identity is preserved"), Model.Edges[BranchEdge].Id, BranchEdgeId);
	TestEqual(TEXT("Branch edge data is preserved"), Model.Edges[BranchEdge].DataId, static_cast<uint32>(33));
	TestTrue(TEXT("Main road still meets the original junction"),
		Model.FindEdge(MainA, Junction) != INDEX_NONE && Model.FindEdge(Junction, MainB) != INDEX_NONE);
	TestTrue(TEXT("Complete branch remains connected through its detached endpoint"),
		Model.FindEdge(Detached, BranchA) != INDEX_NONE && Model.FindEdge(BranchA, BranchB) != INDEX_NONE);
	TestEqual(TEXT("Old branch connection is gone"), Model.FindEdge(Junction, BranchA), INDEX_NONE);

	const FVector OriginalLocation = Model.Vertices[Junction].Transform.GetLocation();
	TestTrue(TEXT("Detach begins without reshaping the branch"),
		Model.Vertices[Detached].Transform.GetLocation().Equals(OriginalLocation, UE_KINDA_SMALL_NUMBER));
	Model.Vertices[Detached].Transform.SetLocation(FVector(0.0, 25.0, 0.0));
	FPCGExClusterSketchValidation Validation;
	Model.Validate(Validation);
	TestEqual(TEXT("A completed drag leaves no collocated vertices"), Validation.CollocatedVertices, 0);

	const int32 VerticesBeforeInvalidCall = Model.NumVertices();
	TestEqual(TEXT("A non-incident endpoint is rejected"), Model.DetachEdgeEndpoint(BranchEdge, MainA), INDEX_NONE);
	TestEqual(TEXT("Rejected detach is non-mutating"), Model.NumVertices(), VerticesBeforeInvalidCall);
	return true;
}

#endif

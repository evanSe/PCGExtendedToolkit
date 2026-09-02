// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Elements/PCGExSpawnDynamicMesh.h"

#include "PCGComponent.h"
#include "PCGExTopology.h"
#include "PCGPin.h"
#include "Components/DynamicMeshComponent.h"
#include "Core/PCGExMT.h"
#include "Data/PCGDynamicMeshData.h"
#include "Data/PCGExDataHelpers.h"
#include "Helpers/PCGExStreamingHelpers.h"
#include "Metadata/PCGMetadata.h"

#define LOCTEXT_NAMESPACE "PCGExGraphSettings"
#define PCGEX_NAMESPACE SpawnDynamicMesh

void PCGExSpawnDynamicMesh::InitializeComponentFromData(
	UDynamicMeshComponent& Component,
	const UPCGDynamicMeshData& MeshData)
{
	// Keep the native PCG Dynamic Mesh contract in one place. In particular, material
	// slots must survive the data-to-component handoff instead of silently falling back
	// to the default gray material.
	MeshData.InitializeDynamicMeshComponentFromData(&Component);
}

#pragma region UPCGSettings interface

TArray<FPCGPinProperties> UPCGExSpawnDynamicMeshSettings::InputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PCGEX_PIN_MESH(PCGExTopology::Labels::SourceMeshLabel, "PCG Dynamic Mesh", Required)
	return PinProperties;
}

TArray<FPCGPinProperties> UPCGExSpawnDynamicMeshSettings::OutputPinProperties() const
{
	TArray<FPCGPinProperties> PinProperties;
	PCGEX_PIN_MESH(PCGExTopology::Labels::SourceMeshLabel, "PCG Dynamic Mesh", Normal)
	return PinProperties;
}

FPCGElementPtr UPCGExSpawnDynamicMeshSettings::CreateElement() const
{
	return MakeShared<FPCGExSpawnDynamicMeshElement>();
}

#pragma endregion

bool FPCGExSpawnDynamicMeshElement::AdvanceWork(FPCGExContext* InContext, const UPCGExSettings* InSettings) const
{
	PCGEX_CONTEXT_AND_SETTINGS(SpawnDynamicMesh)

	AActor* TargetActor = Settings->TargetActor.IsValid() ? Settings->TargetActor.Get() : InContext->GetTargetActor(nullptr);
	if (!TargetActor)
	{
		PCGLog::LogErrorOnGraph(LOCTEXT("InvalidTargetActor", "Invalid target actor."), InContext);
		return true;
	}

	UPCGComponent* SourcePCGComponent = Context->GetMutableComponent();
	const bool bIsPreviewMode = SourcePCGComponent->IsInPreviewMode();

	PCGEX_MAKE_SHARED(TemplateResources, TSet<FSoftObjectPath>)
	Settings->TemplateDescriptor.GetAssetPaths(*TemplateResources.Get());
	if (!TemplateResources->IsEmpty())
	{
		PCGExHelpers::LoadBlocking_AnyThread(TemplateResources, Context);
	}

	// Output creates UObjects (NewObject) and attaches components -- illegal during a package save / GC. Defer (re-tick) until clear.
	PCGEX_DEFER_IF_OBJECT_WORK_BLOCKED

	int32 Index = -1;
	for (const FPCGTaggedData& Input : InContext->InputData.GetInputsByPin(PCGExTopology::Labels::SourceMeshLabel))
	{
		Index++;

		const UPCGDynamicMeshData* DynMeshData = Cast<UPCGDynamicMeshData>(Input.Data);
		if (!DynMeshData)
		{
			PCGLog::InputOutput::LogInvalidInputDataError(InContext);
			continue;
		}

		const FString ComponentName = TEXT("PCGDynamicMeshComponent");
		const EObjectFlags ObjectFlags = (bIsPreviewMode ? RF_Transient : RF_NoFlags);
		UDynamicMeshComponent* DynamicMeshComponent = NewObject<UDynamicMeshComponent>(TargetActor, MakeUniqueObjectName(TargetActor, UDynamicMeshComponent::StaticClass(), FName(ComponentName)), ObjectFlags);

		if (!DynamicMeshComponent)
		{
			continue;
		}

		SourcePCGComponent->IgnoreChangeOriginDuringGenerationWithScope(DynamicMeshComponent, [&]()
		{
			PCGExSpawnDynamicMesh::InitializeComponentFromData(*DynamicMeshComponent, *DynMeshData);
			Settings->TemplateDescriptor.InitComponent(DynamicMeshComponent);
			if (const UPCGMetadata* Metadata = DynMeshData->ConstMetadata())
			{
				const FPCGMetadataDomain* DataDomain = Metadata->GetConstMetadataDomain(PCGMetadataDomainID::Data);
				const FPCGMetadataAttribute<FTransform>* MeshTransformAttribute = DataDomain
					? DataDomain->GetConstTypedAttribute<FTransform>(TEXT("MeshTransform"))
					: nullptr;
				if (MeshTransformAttribute)
				{
					const FTransform MeshTransform =
						PCGExData::Helpers::ReadDataValue<FTransform>(MeshTransformAttribute);
					FTransform ComponentRelativeTransform =
						MeshTransform.GetRelativeTransform(TargetActor->GetActorTransform());
					// Spline To Mesh's origin and basis are world-space, but its scale already
					// carries the source spline's parent-relative frame. Converting that scale by
					// the target actor again applies the inverse parent scale twice.
					ComponentRelativeTransform.SetScale3D(MeshTransform.GetScale3D());
					DynamicMeshComponent->SetRelativeTransform(ComponentRelativeTransform);
				}
			}
		});

		if (!Settings->PropertyOverrideDescriptions.IsEmpty())
		{
			FPCGObjectOverrides DescriptorOverride(DynamicMeshComponent);
			DescriptorOverride.Initialize(Settings->PropertyOverrideDescriptions, DynamicMeshComponent, DynMeshData, InContext);
			if (DescriptorOverride.IsValid() && !DescriptorOverride.Apply(0))
			{
				PCGLog::LogWarningOnGraph(FText::Format(LOCTEXT("FailOverride", "Failed to override descriptor for input {0}"), Index));
			}
		}

		for (const FString& Tag : Input.Tags)
		{
			DynamicMeshComponent->ComponentTags.AddUnique(*Tag);
		}

		Context->AttachManagedComponent(TargetActor, DynamicMeshComponent, Settings->AttachmentRules.GetRules());
		InContext->OutputData.TaggedData.Emplace(Input);
		Context->AddNotifyActor(TargetActor);
	}

	Context->ExecuteOnNotifyActors(Settings->PostProcessFunctionNames);

	return Context->TryComplete(true);
}

#undef LOCTEXT_NAMESPACE
#undef PCGEX_NAMESPACE

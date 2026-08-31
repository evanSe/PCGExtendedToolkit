// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#include "Components/PCGExValencyCageConnectorComponent.h"

#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Core/PCGExValencyConnectorSet.h"
#include "Cages/PCGExValencyCageBase.h"
#include "PCGExValencyEditorSettings.h"

UPCGExValencyCageConnectorComponent::UPCGExValencyCageConnectorComponent()
{
	Mobility = EComponentMobility::Movable;
	bHiddenInGame = true;
	PrimaryComponentTick.bCanEverTick = false;
}

void UPCGExValencyCageConnectorComponent::OnRegister()
{
	Super::OnRegister();

	if (Identifier.IsNone())
	{
		GenerateDefaultIdentifier();
	}
}

void UPCGExValencyCageConnectorComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	RequestCageRebuild();
}

void UPCGExValencyCageConnectorComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	// Only trigger rebuild if the cage itself isn't being destroyed
	if (!bDestroyingHierarchy)
	{
		RequestCageRebuild();
	}

	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

#if WITH_EDITOR
void UPCGExValencyCageConnectorComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// Check PCGEX_ValencyRebuild meta (same pattern as actor base class)
	bool bShouldRebuild = false;
	if (const FProperty* Property = PropertyChangedEvent.Property)
	{
		if (Property->HasMetaData(TEXT("PCGEX_ValencyRebuild")))
		{
			bShouldRebuild = true;
		}
	}
	if (!bShouldRebuild && PropertyChangedEvent.MemberProperty)
	{
		if (PropertyChangedEvent.MemberProperty->HasMetaData(TEXT("PCGEX_ValencyRebuild")))
		{
			bShouldRebuild = true;
		}
	}

	// Transform properties are inherited from USceneComponent and can't carry meta tags
	if (!bShouldRebuild)
	{
		const FName PropertyName = PropertyChangedEvent.GetPropertyName();
		if (PropertyName == USceneComponent::GetRelativeLocationPropertyName() ||
			PropertyName == USceneComponent::GetRelativeRotationPropertyName() ||
			PropertyName == USceneComponent::GetRelativeScale3DPropertyName())
		{
			bShouldRebuild = true;
		}
	}

	// Debounce interactive changes (dragging sliders) to prevent spam
	if (bShouldRebuild && !UPCGExValencyEditorSettings::ShouldAllowRebuild(PropertyChangedEvent.ChangeType))
	{
		bShouldRebuild = false;
	}

	if (bShouldRebuild)
	{
		RequestCageRebuild();
	}
}

void UPCGExValencyCageConnectorComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	// Trigger rebuild after transform changes via standard viewport gizmo
	if (bFinished)
	{
		RequestCageRebuild();
	}
}

bool UPCGExValencyCageConnectorComponent::CanEditChange(const FProperty* InProperty) const
{
	if (!Super::CanEditChange(InProperty))
	{
		return false;
	}

	// Blueprint-defined connectors on instances: lock identity properties
	if (CreationMethod == EComponentCreationMethod::SimpleConstructionScript && !IsTemplate())
	{
		const FName PropName = InProperty->GetFName();
		if (PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, Identifier) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, ConnectorType) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, ConstraintOverrides) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, OverrideMode) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, bInheritable) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, MeshSocketName) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, bMatchMeshSocketTransform) ||
			PropName == GET_MEMBER_NAME_CHECKED(UPCGExValencyCageConnectorComponent, bOverrideAutoExtracted))
		{
			return false;
		}
	}

	return true;
}
#endif

FLinearColor UPCGExValencyCageConnectorComponent::GetEffectiveDebugColor(const UPCGExValencyConnectorSet* ConnectorSet) const
{
	if (DebugColorOverride.A > 0.0f)
	{
		return DebugColorOverride;
	}

	if (ConnectorSet)
	{
		const int32 TypeIndex = ConnectorSet->FindConnectorTypeIndex(ConnectorType);
		if (ConnectorSet->ConnectorTypes.IsValidIndex(TypeIndex))
		{
			return ConnectorSet->ConnectorTypes[TypeIndex].DebugColor;
		}
	}

	return FLinearColor::White;
}

bool UPCGExValencyCageConnectorComponent::SyncTransformFromMeshSocket(UStaticMesh* Mesh)
{
	if (!Mesh || MeshSocketName.IsNone())
	{
		return false;
	}

	const UStaticMeshSocket* MeshSocket = Mesh->FindSocket(MeshSocketName);
	if (!MeshSocket)
	{
		return false;
	}

	const FTransform SocketTransform(
		MeshSocket->RelativeRotation,
		MeshSocket->RelativeLocation,
		MeshSocket->RelativeScale
	);

	SetRelativeTransform(SocketTransform);
	return true;
}

void UPCGExValencyCageConnectorComponent::GenerateDefaultIdentifier()
{
	if (AActor* Owner = GetOwner())
	{
		TArray<UPCGExValencyCageConnectorComponent*> ExistingComponents;
		Owner->GetComponents<UPCGExValencyCageConnectorComponent>(ExistingComponents);

		int32 NextIndex = 0;
		TSet<FName> ExistingIds;
		for (const UPCGExValencyCageConnectorComponent* Comp : ExistingComponents)
		{
			if (Comp && Comp != this)
			{
				ExistingIds.Add(Comp->Identifier);
			}
		}

		FName CandidateId;
		do
		{
			CandidateId = FName(*FString::Printf(TEXT("Connector_%d"), NextIndex++));
		}
		while (ExistingIds.Contains(CandidateId));

		Identifier = CandidateId;
	}
	else
	{
		Identifier = FName("Connector_0");
	}
}

void UPCGExValencyCageConnectorComponent::RequestCageRebuild()
{
	if (APCGExValencyCageBase* Cage = Cast<APCGExValencyCageBase>(GetOwner()))
	{
		Cage->RequestRebuild(EValencyRebuildReason::AssetChange);
	}
}

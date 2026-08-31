// Copyright 2026 Timothé Lapetite and contributors
// Released under the MIT license https://opensource.org/license/MIT/

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/PCGExValencyConnectorSet.h"

#include "PCGExValencyCageConnectorComponent.generated.h"

class UPCGExValencyConnectorSet;
class UStaticMesh;

/**
 * A connector component attached to a Valency cage.
 * Connectors are non-directional connection points that map to orbitals during compilation.
 * Unlike orbitals (which are direction-based), connectors have explicit transforms and type identity.
 */
UCLASS(BlueprintType, ClassGroup = PCGEx, meta = (BlueprintSpawnableComponent), HideCategories = (Mobility, LOD, Collision, Physics, Rendering, Navigation, Cooking))
class PCGEXELEMENTSVALENCYEDITOR_API UPCGExValencyCageConnectorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UPCGExValencyCageConnectorComponent();

	//~ Begin UActorComponent Interface
	virtual void OnRegister() override;
	virtual void OnComponentCreated() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent Interface

	// ========== Connector Properties ==========

	/** Connector identifier (unique per cage, used for socket matching and pipeline output). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (PCGEX_ValencyRebuild))
	FName Identifier;

	/** Connector type (references ConnectorSet.ConnectorTypes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (PCGEX_ValencyRebuild))
	FName ConnectorType;

	/** Connector polarity - determines connection compatibility */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (PCGEX_ValencyRebuild))
	EPCGExConnectorPolarity Polarity = EPCGExConnectorPolarity::Universal;

	/** Whether this connector is enabled (disabled connectors are ignored during compilation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (PCGEX_ValencyRebuild))
	bool bEnabled = true;

	/** Whether this connector is inherited by cages that mirror this cage's connectors */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (PCGEX_ValencyRebuild))
	bool bInheritable = true;

	/** Frontier expansion priority. Higher = expanded sooner. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (PCGEX_ValencyRebuild))
	float Priority = 0.0f;

	/** Max children this connector can spawn. 1 = normal. >1 = multi-spawn. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", meta = (ClampMin = "1", ClampMax = "16", PCGEX_ValencyRebuild))
	int32 SpawnCapacity = 1;

	// ========== Constraints ==========

	/** Per-instance constraint overrides */
	UPROPERTY(EditAnywhere, Category = "Connector|Constraints", meta=(BaseStruct="/Script/PCGExElementsValency.PCGExConnectorConstraint", ExcludeBaseStruct, PCGEX_ValencyRebuild))
	TArray<FInstancedStruct> ConstraintOverrides;

	/** How instance overrides interact with type-level default constraints */
	UPROPERTY(EditAnywhere, Category = "Connector|Constraints", meta=(DisplayAfter="ConstraintOverrides", PCGEX_ValencyRebuild))
	EPCGExConstraintOverrideMode OverrideMode = EPCGExConstraintOverrideMode::Append;

	// ========== Orbital Override ==========

	/** Override automatic orbital index assignment for this connector */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", AdvancedDisplay, meta = (PCGEX_ValencyRebuild))
	bool bManualOrbitalOverride = false;

	/** Manual orbital index (0-63). Only used when bManualOrbitalOverride is true. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector", AdvancedDisplay,
		meta = (EditCondition = "bManualOrbitalOverride", ClampMin = "0", ClampMax = "63", PCGEX_ValencyRebuild))
	int32 ManualOrbitalIndex = 0;

	// ========== Mesh Integration ==========

	/** Optional reference to a mesh socket name to inherit transform from. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector|Mesh Integration", AdvancedDisplay, meta = (PCGEX_ValencyRebuild))
	FName MeshSocketName;

	/** If enabled, automatically match and inherit transform from a mesh socket. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector|Mesh Integration", AdvancedDisplay, meta = (PCGEX_ValencyRebuild))
	bool bMatchMeshSocketTransform = false;

	/** If enabled, this connector component overrides any auto-extracted connector with the same name. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector|Mesh Integration", AdvancedDisplay, meta = (PCGEX_ValencyRebuild))
	bool bOverrideAutoExtracted = true;

	// ========== Visualization ==========

	/** Debug visualization color override. If (0,0,0,0), uses the color from ConnectorSet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Connector|Visualization", AdvancedDisplay)
	FLinearColor DebugColorOverride = FLinearColor(0, 0, 0, 0);

	// ========== Methods ==========

	FLinearColor GetEffectiveDebugColor(const UPCGExValencyConnectorSet* ConnectorSet) const;
	FTransform GetConnectorWorldTransform() const { return GetComponentTransform(); }
	FTransform GetConnectorLocalTransform() const { return GetRelativeTransform(); }
	bool SyncTransformFromMeshSocket(UStaticMesh* Mesh);

protected:
	void GenerateDefaultIdentifier();
	void RequestCageRebuild();
};

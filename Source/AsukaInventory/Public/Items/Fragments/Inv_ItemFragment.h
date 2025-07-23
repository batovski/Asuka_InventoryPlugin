// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Inv_FragmentTags.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_ItemFragment.generated.h"


class AInv_EquipActor;
struct FInv_ConsumeModifier;
class UInv_CompositeBase;
class APlayerController;
/**
 * 
 */
USTRUCT(BlueprintType)
struct FInv_ItemFragment
{
	GENERATED_BODY()

	FInv_ItemFragment() {};
	FInv_ItemFragment(const FInv_ItemFragment& Other) = default;
	FInv_ItemFragment& operator=(const FInv_ItemFragment& Other) = default;
	FInv_ItemFragment(FInv_ItemFragment&& Other) = default; //move constructor
	FInv_ItemFragment& operator=(FInv_ItemFragment&& Other) = default; //move assignment
	virtual ~FInv_ItemFragment(){};

	virtual void Manifest() {}

	FGameplayTag GetFragmentTag() const { return FragmentTag; }
	void SetFragmentTag(FGameplayTag NewTag) { FragmentTag = NewTag; }

	bool IsDynamicFragment() const { return bDynamicFragment; }

protected:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (Categories="FragmentTags"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;

	bool bDynamicFragment{ false }; // If true, this fragment is dynamic and can be modified at runtime will be synced with the item component.
};

USTRUCT(BlueprintType)
struct FInv_GridFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	FInv_GridFragment() {FragmentTag = FragmentTags::GridFragment;}
	FIntPoint GetGridSize() const { return GridSize; }
	void SetGridSize(const FIntPoint& NewSize) { GridSize = NewSize; }
	float GetGridPadding() const { return GridPadding; }
	void SetGridPadding(float NewPadding) { GridPadding = NewPadding; }
private:

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FIntPoint GridSize{ 1, 1 }; // Size of the grid in cells

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float GridPadding{ 0.f };
};

USTRUCT(meta = (HiddenByDefault))
struct FInv_PickUpFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	FInv_PickUpFragment()
	{
		bDynamicFragment = true; // This fragment is dynamic and can be modified at runtime
		FragmentTag = FragmentTags::DynamicFragmentTags::PickUpActorFragment;
	}
	FInv_PickUpFragment(const TSubclassOf<AActor>& NewPickUpClass)
	{
		bDynamicFragment = true; // This fragment is dynamic and can be modified at runtime
		FragmentTag = FragmentTags::DynamicFragmentTags::PickUpActorFragment;
		PickupActorClass = NewPickUpClass;
	}
	TSubclassOf<AActor> GetPickUpActorClass() const { return PickupActorClass; }
	void SetPickUpActorClass(const TSubclassOf<AActor>& NewPickUpClass) { PickupActorClass = NewPickUpClass; }

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TSubclassOf<AActor> PickupActorClass{ nullptr };
};

USTRUCT(BlueprintType)
struct FInv_StackableFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	FInv_StackableFragment()
	{
		bDynamicFragment = true; // This fragment is dynamic and can be modified at runtime
		FragmentTag = FragmentTags::StackableFragment;
	}
	int32 GetMaxStackSize() const { return MaxStackSize; }
	int32 GetStackCount() const { return StackCount; }
	void SetStackCount(int32 Count) { StackCount = Count; }

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxStackSize{ 1 };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 StackCount{ 1 };
};

USTRUCT(meta = (HiddenByDefault))
struct FInv_InventoryItemFragmentAbstract : public FInv_ItemFragment
{
	GENERATED_BODY()

	virtual void Assimilate(UInv_CompositeBase* Composite) const;

protected:
	bool MatchesWidgetTag(const UInv_CompositeBase* Composite) const;
};

USTRUCT(BlueprintType)
struct FInv_LabeledNumberFragment : public FInv_InventoryItemFragmentAbstract
{
	GENERATED_BODY()
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	virtual void Manifest() override;
	float GetValue() const { return Value; }

	bool bRandomizeOnManifest{ true };
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText Text_Label{};
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	float Value{ 0.f };

	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Min{ 0.f };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float Max{ 0.f };

	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseLabel{ false };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bCollapseValue{ false };

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MinFractionalDigits{ 1 };
	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxFractionalDigits{ 1 };
};

USTRUCT(BlueprintType)
struct FInv_ConsumableFragment : public FInv_InventoryItemFragmentAbstract
{
	GENERATED_BODY()
	FInv_ConsumableFragment() { FragmentTag = FragmentTags::ConsumableFragment; }
	virtual void Manifest() override;
	virtual void OnConsume(APlayerController* PC);
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_ConsumeModifier>> ConsumeModifiers;
};

//Consume Fragments :
USTRUCT(BlueprintType)
struct FInv_ConsumeModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY();
	virtual void OnConsume(APlayerController* PC) {}

};

USTRUCT(BlueprintType)
struct FInv_ImageFragment : public FInv_InventoryItemFragmentAbstract
{
	GENERATED_BODY()
	FInv_ImageFragment() { FragmentTag = FragmentTags::IconFragment; }
	UTexture2D* GetIcon() const { return Icon; }
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TObjectPtr<UTexture2D> Icon{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FVector2D IconDimensions{ 44.f };
};

USTRUCT(BlueprintType)
struct FInv_TextFragment : public FInv_InventoryItemFragmentAbstract
{
	GENERATED_BODY()
	FText GetText() const { return FragmentText; }
	void SetText(const FText& NewText) { FragmentText = NewText; }
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;

private:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText FragmentText;
};

USTRUCT(BlueprintType)
struct FInv_HealthPotionFragment : public FInv_ConsumeModifier
{
	GENERATED_BODY()
	virtual void OnConsume(APlayerController* PC) override;
};

// Equipment Fragments :
//

USTRUCT(BlueprintType)
struct FInv_EquipModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) {}
	virtual void OnUnEquip(APlayerController* PC) {}
};

USTRUCT(BlueprintType)
struct FInv_ArmorModifier : public FInv_EquipModifier
{
	GENERATED_BODY()

	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnEquip(APlayerController* PC) override;
};

USTRUCT(BlueprintType)
struct FInv_EquipmentFragment : public FInv_InventoryItemFragmentAbstract
{
	GENERATED_BODY()
	bool bEquipped{ false };
	FInv_EquipmentFragment() { FragmentTag = FragmentTags::EquipmentFragment; }
	virtual void Manifest() override;
	virtual void OnEquip(APlayerController* PC);
	virtual void OnUnEquip(APlayerController* PC);
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;

	AInv_EquipActor* SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const;
	void DestroyAttachedActor() const;
	FGameplayTag GetEquipmentType() const { return EquipmentType; }
	void SetEquippedActor(AInv_EquipActor* NewActor);
private:
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_EquipModifier>> EquipModifiers;
	UPROPERTY(EditAnywhere, Category = "Inventory", meta = (ExcludeBaseStruct))
	TSubclassOf<AInv_EquipActor> EquipActorClass{nullptr};
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FName SocketAttachPoint{NAME_None};
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGameplayTag EquipmentType{ FGameplayTag::EmptyTag };

	TWeakObjectPtr<AInv_EquipActor> EquippedActor{nullptr};
};
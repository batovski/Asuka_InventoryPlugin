// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryManagment/Utils/Inv_InventoryStatics.h"

#include "Engine/AssetManager.h"
#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"
#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "InventoryManagment/Components/Inv_ExternalInventoryComponent.h"
#include "InventoryManagment/Components/Inv_InventoryComponent.h"
#include "InventoryManagment/ItemData/Inv_ItemDataAsset.h"
#include "Items/Inv_InventoryItem.h"
#include "Widgets/Inventory/Base/Inv_InventoryBase.h"

UInv_InventoryComponent* UInv_InventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return nullptr;

	return PlayerController->FindComponentByClass<UInv_InventoryComponent>();

}

UInv_EquipmentComponent* UInv_InventoryStatics::GetEquipmentComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController)) return nullptr;
	return PlayerController->FindComponentByClass<UInv_EquipmentComponent>();
}

void UInv_InventoryStatics::ItemHovered(APlayerController* PC, UInv_InventoryItem* Item)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PC);
	if (!IsValid(IC)) return;
	UInv_InventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return;

	if(InventoryBase->HasHoverItem()) return;

	InventoryBase->OnItemHovered(Item);
}

void UInv_InventoryStatics::ItemUnhovered(APlayerController* PC)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PC);
	if (!IsValid(IC)) return;
	UInv_InventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return;

	InventoryBase->OnItemUnhovered();
}

UInv_HoverItem* UInv_InventoryStatics::GetHoverItem(const APlayerController* PlayerController)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PlayerController);
	if (!IsValid(IC)) return nullptr;
	UInv_InventoryBase* InventoryBase = IC->GetInventoryMenu();
	if (!IsValid(InventoryBase)) return nullptr;

	return InventoryBase->GetHoverItem();
}
FInstancedStruct& UInv_InventoryStatics::GetFragmentFromItem(UInv_InventoryItem* Item, FGameplayTag FragmentType,
	bool& IsFound)
{
	static FInstancedStruct FoundFragment;
	if(!IsValid(Item))
	{
		IsFound = false;
		return FoundFragment;
	}
	if(FInstancedStruct* Fragment = Item->GetFragmentStructByTagMutable(FragmentType))
	{
		IsFound = true;
		return *Fragment;
	}
	IsFound = false;
	return FoundFragment;
}
void UInv_InventoryStatics::SetFragmentFloatProperty(UInv_InventoryItem* Item, FGameplayTag FragmentType,
	const FString& PropertyName, float Value, bool& IsSucceeded)
{
	IsSucceeded = false;
	
	if (!IsValid(Item))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Invalid Item"));
		return;
	}

	if (PropertyName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: PropertyName is empty"));
		return;
	}

	// Get the fragment
	FInstancedStruct* Fragment = Item->GetFragmentStructByTagMutable(FragmentType);
	if (!Fragment)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Fragment not found for tag %s"), *FragmentType.ToString());
		return;
	}

	const UScriptStruct* ScriptStruct = Fragment->GetScriptStruct();
	void* FragmentMemory = Fragment->GetMutableMemory();

	if (!ScriptStruct || !FragmentMemory)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Invalid script struct or memory"));
		return;
	}

	// Check if this is a nested property (contains '.')
	if (PropertyName.Contains(TEXT(".")))
	{
		// Handle nested properties like "Damage.Value"
		TArray<FString> PropertyPath;
		PropertyName.ParseIntoArray(PropertyPath, TEXT("."), true);
		
		void* CurrentMemory = FragmentMemory;
		const UScriptStruct* CurrentStruct = ScriptStruct;
		
		// Navigate through nested properties
		for (int32 i = 0; i < PropertyPath.Num() - 1; ++i)
		{
			const FProperty* Property = CurrentStruct->FindPropertyByName(*PropertyPath[i]);
			if (!Property)
			{
				UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Property '%s' not found in struct '%s'"), 
					*PropertyPath[i], *CurrentStruct->GetName());
				return;
			}

			// Check if this is a struct property
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				CurrentMemory = StructProperty->ContainerPtrToValuePtr<void>(CurrentMemory);
				CurrentStruct = StructProperty->Struct;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Property '%s' is not a struct, cannot navigate further"), 
					*PropertyPath[i]);
				return;
			}
		}
		
		// Now set the final property
		const FString& FinalPropertyName = PropertyPath.Last();
		const FFloatProperty* FloatProperty = CastField<FFloatProperty>(CurrentStruct->FindPropertyByName(*FinalPropertyName));
		
		if (!FloatProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Final property '%s' is not a float property"), *FinalPropertyName);
			return;
		}
		FloatProperty->SetPropertyValue_InContainer(CurrentMemory, Value);
		IsSucceeded = true;
	}
	else
	{
		// Handle direct properties
		const FFloatProperty* FloatProperty = CastField<FFloatProperty>(ScriptStruct->FindPropertyByName(*PropertyName));
		
		if (!FloatProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetFragmentFloatProperty: Property '%s' not found or is not a float property"), *PropertyName);
			return;
		}
		
		FloatProperty->SetPropertyValue_InContainer(FragmentMemory, Value);
		IsSucceeded = true;
	}

	// Force replication: We need to touch the DynamicItemFragments array to trigger replication
	if (IsSucceeded)
	{
		Item->MarkDynamicFragmentDirty(Fragment->GetPtr<FInv_ItemFragment>());
	}
}

void UInv_InventoryStatics::SetFragmentIntProperty(UInv_InventoryItem* Item, FGameplayTag FragmentType,
	const FString& PropertyName, int32 Value, bool& IsSucceeded)
{
	IsSucceeded = false;

	if (!IsValid(Item))
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Invalid Item"));
		return;
	}

	if (PropertyName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: PropertyName is empty"));
		return;
	}

	// Get the fragment
	FInstancedStruct* Fragment = Item->GetFragmentStructByTagMutable(FragmentType);
	if (!Fragment)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Fragment not found for tag %s"), *FragmentType.ToString());
		return;
	}

	const UScriptStruct* ScriptStruct = Fragment->GetScriptStruct();
	void* FragmentMemory = Fragment->GetMutableMemory();

	if (!ScriptStruct || !FragmentMemory)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Invalid script struct or memory"));
		return;
	}

	// Check if this is a nested property (contains '.')
	if (PropertyName.Contains(TEXT(".")))
	{
		// Handle nested properties like "Stats.Count"
		TArray<FString> PropertyPath;
		PropertyName.ParseIntoArray(PropertyPath, TEXT("."), true);

		void* CurrentMemory = FragmentMemory;
		const UScriptStruct* CurrentStruct = ScriptStruct;

		// Navigate through nested properties
		for (int32 i = 0; i < PropertyPath.Num() - 1; ++i)
		{
			const FProperty* Property = CurrentStruct->FindPropertyByName(*PropertyPath[i]);
			if (!Property)
			{
				UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Property '%s' not found in struct '%s'"),
					*PropertyPath[i], *CurrentStruct->GetName());
				return;
			}

			// Check if this is a struct property
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				CurrentMemory = StructProperty->ContainerPtrToValuePtr<void>(CurrentMemory);
				CurrentStruct = StructProperty->Struct;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Property '%s' is not a struct, cannot navigate further"),
					*PropertyPath[i]);
				return;
			}
		}

		// Now set the final property
		const FString& FinalPropertyName = PropertyPath.Last();
		const FIntProperty* IntProperty = CastField<FIntProperty>(CurrentStruct->FindPropertyByName(*FinalPropertyName));

		if (!IntProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Final property '%s' is not an int property"), *FinalPropertyName);
			return;
		}
		IntProperty->SetPropertyValue_InContainer(CurrentMemory, Value);
		IsSucceeded = true;
	}
	else
	{
		// Handle direct properties
		const FIntProperty* IntProperty = CastField<FIntProperty>(ScriptStruct->FindPropertyByName(*PropertyName));

		if (!IntProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("SetFragmentIntProperty: Property '%s' not found or is not an int property"), *PropertyName);
			return;
		}

		IntProperty->SetPropertyValue_InContainer(FragmentMemory, Value);
		IsSucceeded = true;
	}

	// Force replication: We need to touch the DynamicItemFragments array to trigger replication
	if (IsSucceeded)
	{
		Item->MarkDynamicFragmentDirty(Fragment->GetPtr<FInv_ItemFragment>());
	}
}

float UInv_InventoryStatics::GetFragmentFloatProperty(UInv_InventoryItem* Item, FGameplayTag FragmentType,
	const FString& PropertyName, bool& IsSucceeded)
{
	IsSucceeded = false;
	float DefaultValue = 0.0f;
	
	if (!IsValid(Item))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Invalid Item"));
		return DefaultValue;
	}

	if (PropertyName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: PropertyName is empty"));
		return DefaultValue;
	}

	// Get the fragment (const version since we're only reading)
	const FInstancedStruct* Fragment = nullptr;
	if (FInstancedStruct* MutableFragment = Item->GetFragmentStructByTagMutable(FragmentType))
	{
		Fragment = MutableFragment;
	}
	
	if (!Fragment)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Fragment not found for tag %s"), *FragmentType.ToString());
		return DefaultValue;
	}

	const UScriptStruct* ScriptStruct = Fragment->GetScriptStruct();
	const void* FragmentMemory = Fragment->GetMemory();

	if (!ScriptStruct || !FragmentMemory)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Invalid script struct or memory"));
		return DefaultValue;
	}

	// Check if this is a nested property (contains '.')
	if (PropertyName.Contains(TEXT(".")))
	{
		// Handle nested properties like "Damage.Value"
		TArray<FString> PropertyPath;
		PropertyName.ParseIntoArray(PropertyPath, TEXT("."), true);
		
		const void* CurrentMemory = FragmentMemory;
		const UScriptStruct* CurrentStruct = ScriptStruct;
		
		// Navigate through nested properties
		for (int32 i = 0; i < PropertyPath.Num() - 1; ++i)
		{
			const FProperty* Property = CurrentStruct->FindPropertyByName(*PropertyPath[i]);
			if (!Property)
			{
				UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Property '%s' not found in struct '%s'"), 
					*PropertyPath[i], *CurrentStruct->GetName());
				return DefaultValue;
			}

			// Check if this is a struct property
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				CurrentMemory = StructProperty->ContainerPtrToValuePtr<const void>(CurrentMemory);
				CurrentStruct = StructProperty->Struct;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Property '%s' is not a struct, cannot navigate further"), 
					*PropertyPath[i]);
				return DefaultValue;
			}
		}
		
		// Now get the final property value
		const FString& FinalPropertyName = PropertyPath.Last();
		const FFloatProperty* FloatProperty = CastField<FFloatProperty>(CurrentStruct->FindPropertyByName(*FinalPropertyName));
		
		if (!FloatProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Final property '%s' is not a float property"), *FinalPropertyName);
			return DefaultValue;
		}
		
		float Value = FloatProperty->GetPropertyValue_InContainer(CurrentMemory);
		IsSucceeded = true;
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("GetFragmentFloatProperty: Successfully read property '%s' = %f from fragment '%s'"), 
			*PropertyName, Value, *FragmentType.ToString());
			
		return Value;
	}
	else
	{
		// Handle direct properties
		const FFloatProperty* FloatProperty = CastField<FFloatProperty>(ScriptStruct->FindPropertyByName(*PropertyName));
		
		if (!FloatProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetFragmentFloatProperty: Property '%s' not found or is not a float property"), *PropertyName);
			return DefaultValue;
		}
		
		float Value = FloatProperty->GetPropertyValue_InContainer(FragmentMemory);
		IsSucceeded = true;
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("GetFragmentFloatProperty: Successfully read property '%s' = %f from fragment '%s'"), 
			*PropertyName, Value, *FragmentType.ToString());
			
		return Value;
	}
}

int32 UInv_InventoryStatics::GetFragmentIntProperty(UInv_InventoryItem* Item, FGameplayTag FragmentType,
	const FString& PropertyName, bool& IsSucceeded)
{
	IsSucceeded = false;
	int32 DefaultValue = 0;
	
	if (!IsValid(Item))
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Invalid Item"));
		return DefaultValue;
	}

	if (PropertyName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: PropertyName is empty"));
		return DefaultValue;
	}

	// Get the fragment (const version since we're only reading)
	const FInstancedStruct* Fragment = nullptr;
	if (FInstancedStruct* MutableFragment = Item->GetFragmentStructByTagMutable(FragmentType))
	{
		Fragment = MutableFragment;
	}
	
	if (!Fragment)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Fragment not found for tag %s"), *FragmentType.ToString());
		return DefaultValue;
	}

	const UScriptStruct* ScriptStruct = Fragment->GetScriptStruct();
	const void* FragmentMemory = Fragment->GetMemory();

	if (!ScriptStruct || !FragmentMemory)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Invalid script struct or memory"));
		return DefaultValue;
	}

	// Check if this is a nested property (contains '.')
	if (PropertyName.Contains(TEXT(".")))
	{
		// Handle nested properties like "Stats.Count"
		TArray<FString> PropertyPath;
		PropertyName.ParseIntoArray(PropertyPath, TEXT("."), true);
		
		const void* CurrentMemory = FragmentMemory;
		const UScriptStruct* CurrentStruct = ScriptStruct;
		
		// Navigate through nested properties
		for (int32 i = 0; i < PropertyPath.Num() - 1; ++i)
		{
			const FProperty* Property = CurrentStruct->FindPropertyByName(*PropertyPath[i]);
			if (!Property)
			{
				UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Property '%s' not found in struct '%s'"), 
					*PropertyPath[i], *CurrentStruct->GetName());
				return DefaultValue;
			}

			// Check if this is a struct property
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
			{
				CurrentMemory = StructProperty->ContainerPtrToValuePtr<const void>(CurrentMemory);
				CurrentStruct = StructProperty->Struct;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Property '%s' is not a struct, cannot navigate further"), 
					*PropertyPath[i]);
				return DefaultValue;
			}
		}
		
		// Now get the final property value
		const FString& FinalPropertyName = PropertyPath.Last();
		const FIntProperty* IntProperty = CastField<FIntProperty>(CurrentStruct->FindPropertyByName(*FinalPropertyName));
		
		if (!IntProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Final property '%s' is not an int property"), *FinalPropertyName);
			return DefaultValue;
		}
		
		const int32 Value = IntProperty->GetPropertyValue_InContainer(CurrentMemory);
		IsSucceeded = true;
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("GetFragmentIntProperty: Successfully read property '%s' = %d from fragment '%s'"), 
			*PropertyName, Value, *FragmentType.ToString());
			
		return Value;
	}
	else
	{
		// Handle direct properties
		const FIntProperty* IntProperty = CastField<FIntProperty>(ScriptStruct->FindPropertyByName(*PropertyName));
		
		if (!IntProperty)
		{
			UE_LOG(LogTemp, Warning, TEXT("GetFragmentIntProperty: Property '%s' not found or is not an int property"), *PropertyName);
			return DefaultValue;
		}
		
		const int32 Value = IntProperty->GetPropertyValue_InContainer(FragmentMemory);
		IsSucceeded = true;
		
		UE_LOG(LogTemp, VeryVerbose, TEXT("GetFragmentIntProperty: Successfully read property '%s' = %d from fragment '%s'"), 
			*PropertyName, Value, *FragmentType.ToString());
			
		return Value;
	}
}
const UInv_InventoryItem* UInv_InventoryStatics::GetInventoryItemFromPlayerController(const APlayerController* PlayerController, FGameplayTag ItemType)
{
	const UInv_InventoryComponent* IC = GetInventoryComponent(PlayerController);
	if (!IsValid(IC)) return nullptr;
	const UInv_InventoryItem* Item = IC->FindInventoryItem(ItemType);
	if (IsValid(Item)) return Item;
	const UInv_EquipmentComponent* EC = GetEquipmentComponent(PlayerController);
	if (!IsValid(EC)) return nullptr;
	Item = EC->Execute_FindFirstItemByType(EC, ItemType);
	if (IsValid(Item)) return Item;
	return nullptr;
}

const UInv_InventoryItem* UInv_InventoryStatics::GetEquipmentItemFromPlayerController(
	const APlayerController* PlayerController, FGameplayTag ItemType)
{
	if (const AInv_EquipActor* ItemActor = GetEquipmentActorFromPlayerController(PlayerController, ItemType))
	{
		return ItemActor->GetOwningItem();
	}
	return nullptr;
}

const AInv_EquipActor* UInv_InventoryStatics::GetEquipmentActorFromPlayerController(
	const APlayerController* PlayerController, FGameplayTag ItemType)
{
	check(IsValid(PlayerController));
	if (UInv_EquipmentComponent* EquipmentComponent = PlayerController->FindComponentByClass<UInv_EquipmentComponent>())
	{
		if (AInv_EquipActor* ItemActor = EquipmentComponent->FindEquippedActorByType(ItemType))
		{
			return ItemActor;
		}
	}
	return nullptr;
}

UInv_InventoryBase* UInv_InventoryStatics::GetInventoryWidget(const APlayerController* PlayerController)
{
	UInv_InventoryComponent* IC = GetInventoryComponent(PlayerController);
	if (!IsValid(IC)) return nullptr;
	return IC->GetInventoryMenu();
}

FInv_ItemManifest UInv_InventoryStatics::GetItemManifestFromID(const FPrimaryAssetId& ItemId)
{
	UAssetManager& AssetManager = UAssetManager::Get();
	UObject* Asset = AssetManager.GetPrimaryAssetObject(ItemId);
	if (!IsValid(Asset))
	{
		auto Handle = AssetManager.LoadPrimaryAsset(ItemId);
		if (Handle)
		{
			Handle->WaitUntilComplete();
			Asset = Handle->GetLoadedAsset();
		}
	}
	return Asset ? Cast<UInv_ItemDataAsset>(Asset)->ItemManifest : FInv_ItemManifest();
}

UInv_InventoryItem* UInv_InventoryStatics::CreateInventoryItemFromManifest(const FPrimaryAssetId& ItemId, UObject* WorldContextObject, const TArray<FInstancedStruct>& DynamicFragments)
{
	UInv_InventoryItem* NewItem = NewObject<UInv_InventoryItem>(WorldContextObject, UInv_InventoryItem::StaticClass());;
	NewItem->SetStaticItemManifestAssetId(ItemId);
	NewItem->LoadStaticItemManifest();
	if(!DynamicFragments.IsEmpty())
	{
		NewItem->SetDynamicItemFragments(DynamicFragments);
	}
	NewItem->InitManifestDynamicFragments(WorldContextObject);
	return NewItem;
}

AActor* UInv_InventoryStatics::CreateExternalInventoryActor(UObject* WorldContextObject,
	UInv_InventoryComponent* AuthoritativeInventoryComponent,
	TArray<TScriptInterface<IInv_ItemListInterface>> InventoryComponentList,
	FInv_ExternalInventorySpawnParams InventorySpawnParams,
    TSubclassOf<AActor> ActorClass,
    const FTransform& SpawnTransform)
{
    if (!ActorClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateExternalInventoryComponent: ActorClass is null"));
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    // Spawn the actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);
    if (!SpawnedActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("CreateExternalInventoryComponent: Failed to spawn actor of class %s"), 
               ActorClass ? *ActorClass->GetName() : TEXT("NULL"));
        return nullptr;
    }

    // Create and attach the external inventory component
    UInv_ExternalInventoryComponent* ExternalInventoryComponent = NewObject<UInv_ExternalInventoryComponent>(
        SpawnedActor, UInv_ExternalInventoryComponent::StaticClass());
    
    if (!ExternalInventoryComponent)
    {
        SpawnedActor->Destroy();
        return nullptr;
    }

    // Add component to the actor
    SpawnedActor->AddInstanceComponent(ExternalInventoryComponent);
    ExternalInventoryComponent->RegisterComponent();

    // Set pickup message if provided
    if (!InventorySpawnParams.PickupMessage.IsEmpty())
    {
        ExternalInventoryComponent->GetPickupMessage() = InventorySpawnParams.PickupMessage;
    }
	ExternalInventoryComponent->Columns = InventorySpawnParams.Columns;
	ExternalInventoryComponent->Rows = InventorySpawnParams.Rows;
	ExternalInventoryComponent->GridNameText = InventorySpawnParams.GridNameText;
	ExternalInventoryComponent->GetGridTagMutable() = InventorySpawnParams.GridEntityTag;

    // Copy items from source inventory component
	for (auto InventoryComponent : InventoryComponentList)
	{
		if (!IsValid(InventoryComponent.GetObject())) continue;
		const TArray<UInv_InventoryItem*>& SourceItems = InventoryComponent->GetInventoryListMutable().GetAllItems();
		for (UInv_InventoryItem* SourceItem : SourceItems)
		{
			if (IsValid(SourceItem))
			{
				FInv_ItemAddingOptions ItemToEquipOptions;
				ItemToEquipOptions.GridIndex = INDEX_NONE;
				AuthoritativeInventoryComponent->Server_AddNewItem(InventoryComponent, ExternalInventoryComponent, SourceItem, ItemToEquipOptions);
			}
		}
	}
    return SpawnedActor;
}

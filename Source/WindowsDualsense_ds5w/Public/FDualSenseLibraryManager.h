﻿// Copyright (c) 2025 Rafael Valoto/Publisher. All rights reserved.
// Created for: WindowsDualsense_ds5w - Plugin to support DualSense controller on Windows.
// Planned Release Year: 2025

#pragma once

#include "CoreMinimal.h"
#include "DualSenseHIDManager.h"
#include "DualSenseLibrary.h"
#include "UObject/Object.h"
#include "Misc/CoreDelegates.h"
#include "FDualSenseLibraryManager.generated.h"

#define MAX_DEVICES 16


/**
 * 
 */
UCLASS()
class WINDOWSDUALSENSE_DS5W_API UFDualSenseLibraryManager : public UObject
{
	GENERATED_BODY()

public:
	
	virtual ~UFDualSenseLibraryManager() override
	{
		UE_LOG(LogTemp, Log, TEXT("DualSense: Destruct UFDualSenseLibraryManager"));
	}
	
	static UFDualSenseLibraryManager* Get()
	{
		if (!Instance)
		{
			UE_LOG(LogTemp, Log, TEXT("DualSense: Creating new instance UFDualSenseLibraryManager"));
			Instance = NewObject<UFDualSenseLibraryManager>();
			Instance->AddToRoot();
		}
		return Instance;
	}
	
	static UDualSenseLibrary* GetLibraryOrRecconect(int32 ControllerId)
	{
		if (LibraryInstances.Contains(ControllerId))
		{
			if (LibraryInstances[ControllerId]->IsConnected())
			{
				return LibraryInstances[ControllerId];
			}
		
			RemoveLibraryInstance(ControllerId); // destruct instance to reconnect
		}
		
		if (!LibraryInstances.Contains(ControllerId))
		{
			UDualSenseLibrary* DSLibrary = CreateLibraryInstance(ControllerId);
			if (!DSLibrary)
			{
				return nullptr;
			}

			UE_LOG(LogTemp, Log, TEXT("DualSense: CreateLibraryInstance UDualSenseLibrary ControllerId, %d"), ControllerId);
			LibraryInstances.Add(ControllerId, DSLibrary);
		}

		LibraryInstances[ControllerId]->Reconnect();
		return LibraryInstances[ControllerId];
	}
	
	static UDualSenseLibrary* GetLibraryInstance(int32 ControllerId)
	{
		if (!LibraryInstances.Contains(ControllerId))
		{
			return nullptr;
		}

		if (!LibraryInstances[ControllerId]->IsConnected())
		{
			return nullptr;
		}
		
		return LibraryInstances[ControllerId];
	}

	static void RemoveLibraryInstance(int32 ControllerId)
	{
		if (LibraryInstances.Contains(ControllerId))
		{
			LibraryInstances[ControllerId]->ShutdownLibrary();
			LibraryInstances.Remove(ControllerId);
			UE_LOG(LogTemp, Log, TEXT("DualSense: ShutdownLibrary UDualSenseLibrary ControllerId, %d"), ControllerId);
		}
	}

	static void RemoveAllLibraryInstance()
	{
		for (const auto& LibraryInstance : LibraryInstances)
		{
			LibraryInstance.Value->ShutdownLibrary();
		}
		
		LibraryInstances.Reset();
	}


	static void CreateLibraryInstances()
	{
		LibraryInstances.Reset();
		
		TArray<FHIDDeviceContext> DetectedDevices;
		DetectedDevices.Reset();

		if (DualSenseHIDManager HIDManager; !HIDManager.FindDevices(DetectedDevices) || DetectedDevices.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("DualSense: device not found. Creating default library instance."));
			return;
		}

		if (DetectedDevices.Num() > MAX_DEVICES)
		{
			return;
		}

		for (int32 DeviceIndex = 0; DeviceIndex < DetectedDevices.Num(); DeviceIndex++)
		{
			FHIDDeviceContext& Context = DetectedDevices[DeviceIndex];
			if (Context.Internal.Connected)
			{
				UE_LOG(LogTemp, Log, TEXT("DualSense: init device isConnected %d"), Context.Internal.Connected);
				
				UDualSenseLibrary* DualSense = NewObject<UDualSenseLibrary>();
				if (!DualSense)
				{
					UE_LOG(LogTemp, Warning, TEXT("DualSense: not found device shutdown library... %d"), DeviceIndex);
					DualSense->ShutdownLibrary();
					continue;
				}

				DualSense->AddToRoot();
				DualSense->ControllerID = DeviceIndex;
				DualSense->InitializeLibrary(Context);

				LibraryInstances.Add(DeviceIndex, DualSense);
				UE_LOG(LogTemp, Log, TEXT("DualSense: Library initialized deviceId %d"), DeviceIndex);
			}
		}
	}

	static int32 GetAllocatedDevices()
	{
		return LibraryInstances.Num();
	}

	static FGenericPlatformInputDeviceMapper PlatformInputDeviceMapper;
	
private:
	static UFDualSenseLibraryManager* Instance;
	static TMap<int32, UDualSenseLibrary*> LibraryInstances;
	
	static UDualSenseLibrary* CreateLibraryInstance(int32 ControllerID)
	{
		TArray<FHIDDeviceContext> DetectedDevices;
		DetectedDevices.Reset();
		
		if (DualSenseHIDManager HIDManager; !HIDManager.FindDevices(DetectedDevices) || DetectedDevices.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("DualSense: device not found. Creating default library instance."));
			return nullptr;
		}

			
		FHIDDeviceContext& Context = DetectedDevices[ControllerID];
		UE_LOG(LogTemp, Log, TEXT("DualSense: init device isConnected %d"), Context.Internal.Connected);
			
		if (Context.Internal.Connected)
		{
			UDualSenseLibrary* DualSense = NewObject<UDualSenseLibrary>();
			if (!DualSense)
			{
				return nullptr;
			}

			DualSense->AddToRoot();
			DualSense->ControllerID = ControllerID;
			DualSense->InitializeLibrary(Context);
			return DualSense;
		}
		return nullptr; 
	}
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponActor.generated.h"

UCLASS()
class NETWORKPROJECT_API AWeaponActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category="My Settings")
	class UBoxComponent* BoxCollision;
	UPROPERTY(EditDefaultsOnly, Category="My Settings")
	class USkeletalMeshComponent* MeshComponent;
	UPROPERTY(EditDefaultsOnly, Replicated, Category="My Settings")
	int32 Ammo;
	UPROPERTY(EditDefaultsOnly, Replicated, Category="My Settings")
	float ReloadTime;
	UPROPERTY(EditDefaultsOnly, Replicated, Category="My Settings")
	float DamagePower;


	UFUNCTION()
	void OnWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Server, Unreliable)
	void ServerGrabWeapon(class ANetworkProjectCharacter* player);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastGrabWeapon(class ANetworkProjectCharacter* player);

	UFUNCTION(Server, Unreliable)
	void ServerReleaseWeapon(class ANetworkProjectCharacter* player);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastReleaseWeapon(class ANetworkProjectCharacter* player);
};

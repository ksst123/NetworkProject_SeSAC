// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BulletActor.generated.h"

UCLASS()
class NETWORKPROJECT_API ABulletActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABulletActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, Category="BulletSettings")
	class USphereComponent* SphereCollision;
	UPROPERTY(EditDefaultsOnly, Category="BulletSettings")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category="BulletSettings")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, Category="BulletSettings")
	int32 AttackPower;

	UPROPERTY(EditAnywhere, Category="BulletSettings")
	class UParticleSystem* FireEffect;


	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	virtual void Destroyed() override;
};

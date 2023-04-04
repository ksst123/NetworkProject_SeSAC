// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OwnershipActor.generated.h"

UCLASS()
class NETWORKPROJECT_API AOwnershipActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOwnershipActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category="My Actor")
	class UBoxComponent* BoxCollision;
	UPROPERTY(EditAnywhere, Category = "My Actor")
	class UStaticMeshComponent* MeshComponent;

	UFUNCTION()
	void ScreenLog();
	UFUNCTION()
	void ChangeColor();
	UFUNCTION()
	void OnOwnershipBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	void OnOwnershipEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	UPROPERTY(Replicated)
	FVector MoveDirection;
	UPROPERTY(ReplicatedUsing=ScreenLog)
	int32 TestNumber = 0;
	// 반환자료형과 매개변수가 없는 함수만 ReplicatedUsing을 사용할 수 있다
	UPROPERTY(ReplicatedUsing = ChangeColor)
	FVector MatColor;

	class UMaterialInstanceDynamic* Mat;

	FString PrintInfo();
	void CheckOwner();
	
};

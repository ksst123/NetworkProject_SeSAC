// Fill out your copyright notice in the Description page of Project Settings.


#include "BulletActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "NetworkProject/NetworkProjectCharacter.h"
#include "Particles/ParticleSystem.h"

// Sets default values
ABulletActor::ABulletActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Collision"));
	SetRootComponent(SphereCollision);
	SphereCollision->SetSphereRadius(15.f);
	SphereCollision->SetCollisionProfileName(FName("BulletPreset"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetRelativeScale3D(FVector(0.3f));
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ABulletActor::BeginPlay()
{
	Super::BeginPlay();

	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ABulletActor::OnOverlap);
}

// Called every frame
void ABulletActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetActorLocation(GetActorLocation() + GetActorForwardVector() * MoveSpeed * DeltaTime);
}


void ABulletActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(GetOwner() == nullptr)
	{
		return;
	}

	ANetworkProjectCharacter* enemy = Cast<ANetworkProjectCharacter>(OtherActor);

	if(OtherActor != GetOwner())
	{
		if (HasAuthority())
		{
			if(enemy != nullptr)
			{
				if(enemy->GetHealth() <= AttackPower)
				{
					ANetworkProjectCharacter* myOwner = Cast<ANetworkProjectCharacter>(GetOwner());
					if(myOwner != nullptr)
					{
						myOwner->GetPlayerState()->SetScore(myOwner->GetPlayerState()->GetScore() + 10);
					}
				}

				enemy->ServerDamage(AttackPower);
				Destroy();
			}
		}
	}
}

void ABulletActor::Destroyed()
{
	Super::Destroyed();

	UParticleSystemComponent* fire = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireEffect, GetActorLocation(), GetActorRotation(), true);
}
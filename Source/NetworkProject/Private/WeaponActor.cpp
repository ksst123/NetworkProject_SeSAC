// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponActor.h"

#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "NetworkProject/NetworkProjectCharacter.h"

// Sets default values
AWeaponActor::AWeaponActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	SetRootComponent(BoxCollision);
	BoxCollision->SetCollisionProfileName(FName("WeaponPreset"));
	BoxCollision->SetSimulatePhysics(true);

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void AWeaponActor::BeginPlay()
{
	Super::BeginPlay();

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeaponActor::OnWeaponBeginOverlap);
}

// Called every frame
void AWeaponActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponActor::OnWeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 만약 서버라면
	if(HasAuthority())
	{
		// 만약, 총을 쥔 플레이어가 아니라면
		ANetworkProjectCharacter* player = Cast<ANetworkProjectCharacter>(OtherActor);
		if(player != nullptr && player->OwningWeapon == nullptr)
		{
			player->CurrentAmmo = Ammo;
			ServerGrabWeapon(player);
		}
	}
}

void AWeaponActor::MulticastReleaseWeapon_Implementation(ANetworkProjectCharacter* player)
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	BoxCollision->SetSimulatePhysics(true);
	player->OwningWeapon = nullptr;
	FTimerHandle ReleaseDelayHandle;
	GetWorldTimerManager().SetTimer(ReleaseDelayHandle, FTimerDelegate::CreateLambda([&]()->void
		{
			BoxCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);

		}), 3.f, false);

}

void AWeaponActor::ServerReleaseWeapon_Implementation(ANetworkProjectCharacter* player)
{
	MulticastReleaseWeapon(player);
	SetOwner(nullptr);
	player->CurrentAmmo = 0;
}

void AWeaponActor::ServerGrabWeapon_Implementation(class ANetworkProjectCharacter* player)
{
	SetOwner(player);
	MulticastGrabWeapon(player);
}

void AWeaponActor::MulticastGrabWeapon_Implementation(class ANetworkProjectCharacter* player)
{
	player->OwningWeapon = this;
	BoxCollision->SetSimulatePhysics(false);
	// 피직스가 켜져있으면 AttachToComponent가 적용되지 않는다.
	AttachToComponent(player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("WeaponSocket"));
	BoxCollision->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Ignore);
}

void AWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeaponActor, Ammo);
	DOREPLIFETIME(AWeaponActor, ReloadTime);
	DOREPLIFETIME(AWeaponActor, DamagePower);
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "OwnershipActor.h"

#include "EngineUtils.h"
#include "Components/BoxComponent.h"
#include "Net/UnrealNetwork.h"
#include "NetworkProject/NetworkProjectCharacter.h"

// Sets default values
AOwnershipActor::AOwnershipActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Box Collision"));
	SetRootComponent(BoxCollision);
	BoxCollision->SetBoxExtent(FVector(50.f));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// ���� ���� on/off �Լ�
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AOwnershipActor::BeginPlay()
{
	Super::BeginPlay();

	// �޽��� ��Ƽ������ ���̳��� ��Ƽ���� �ν��Ͻ��� �����Ѵ�.
	UMaterialInterface* baseMaterial = MeshComponent->GetMaterial(0);
	if(baseMaterial != nullptr)
	{
		Mat = UMaterialInstanceDynamic::Create(baseMaterial, this);
		MeshComponent->SetMaterial(0, Mat);
	}

	// ���� ���� ���Ͷ��
	// if(GetLocalRole() == ROLE_Authority)
	if(HasAuthority())
	{
		// ������ ������ �����Ѵ�.
		MoveDirection = FMath::VRand();
		MoveDirection.Z = FMath::Abs(MoveDirection.Z);

		// 5�ʿ� �� ���� TestNumber�� ������ �����Ѵ�.
		FTimerHandle repeatTimer;
		GetWorldTimerManager().SetTimer(repeatTimer, FTimerDelegate::CreateLambda([&]()->void
			{
				TestNumber = FMath::Rand();
			}
		), 5.f, true);
	}

	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AOwnershipActor::OnOwnershipBeginOverlap);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AOwnershipActor::OnOwnershipEndOverlap);
}

// Called every frame
void AOwnershipActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ���� ������ ����Ѵ�.
	DrawDebugString(GetWorld(), GetActorLocation(), PrintInfo(), nullptr, FColor::White, 0.0f, true);

	// SetActorLocation(GetActorLocation() + MoveDirection * 100.f * DeltaTime);
	if(HasAuthority())
	{
		CheckOwner();
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), 500.f, 30, FColor::Cyan, false, 0.0f, 0, 1);
}

void AOwnershipActor::ScreenLog()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::FromInt(TestNumber), true, FVector2D(1.5f));
}



void AOwnershipActor::OnOwnershipBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(HasAuthority())
	{
		// �浹�� ����� �÷��̾���
		if (OtherActor->IsA<ANetworkProjectCharacter>())
		{
			// ��Ƽ���� ������ �����ϰ� �����Ѵ�.
			MatColor = FMath::VRand();
			MatColor = MatColor.GetAbs();
			ChangeColor();
		}
	}
}

void AOwnershipActor::OnOwnershipEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

FString AOwnershipActor::PrintInfo()
{
#pragma region RoleInfo
	FString myLocalRole = UEnum::GetValueAsString<ENetRole>(GetLocalRole());
	FString myRemoteRole = UEnum::GetValueAsString<ENetRole>(GetRemoteRole());
	FString myConnetion = GetNetConnection() != nullptr ? TEXT("Valid") : TEXT("Invalid");
	FString myOwner = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");
	FString name = this->GetName();

	FString infoText = FString::Printf(TEXT("Local Role: %s\nRemote Role: %s\nNet Connection: %s\nOwner: %s\nName: %s"), *myLocalRole, *myRemoteRole, *myConnetion, *myOwner, *name);
#pragma endregion

#pragma region RandomVector
	// FString infoText = FString::Printf(TEXT("%.2f, %.2f, %.2f"), MoveDirection.X, MoveDirection.Y, MoveDirection.Z);

#pragma endregion

	return infoText;
}

// ������ ���� �ȿ� �÷��̾ �ִٸ� ���� ����� �÷��̾� ĳ���͸� Owner�� �����Ѵ�.
void AOwnershipActor::CheckOwner()
{
	float maxDistance = 500.f;
	ANetworkProjectCharacter* player = nullptr;
	/*TArray<ANetworkProjectCharacter*> players;
	TArray<int32> distances;

	for(TActorIterator<ANetworkProjectCharacter> itr(GetWorld()); itr; ++itr)
	{
		if(GetDistanceTo(*itr) <= maxDistance)
		{
			players.Add(*itr);
			distances.Add(GetDistanceTo(*itr));
		}
	}

	int temp;
	int idx;
	if(distances.Num() != 0)
	{
		temp = distances[0];
		idx = 0;

		for (int i = 1; i < distances.Num(); i++)
		{
			if (temp > distances[i])
			{
				idx = i;
			}
		}

		player = players[idx];
		SetOwner(player);
	}*/

	// ����� Ǯ��
	// ============================================================================
	for (TActorIterator<ANetworkProjectCharacter> itr(GetWorld()); itr; ++itr)
	{
		float distance = this->GetDistanceTo(*itr);

		if(distance < maxDistance)
		{
			maxDistance = distance;
			player = *itr;
		}
	}

	if(player != nullptr && GetOwner() != player)
	{
		SetOwner(player);
	}
	// ============================================================================
}

void AOwnershipActor::ChangeColor()
{
	// "MyColor" �Ķ������ ���� MatColor ������ �����Ѵ�.
	Mat->SetVectorParameterValue(FName("MyColor"), (FLinearColor)MatColor);
}

// ������ ���� ����ϱ� ���� �Լ�
void AOwnershipActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOwnershipActor, MoveDirection);
	DOREPLIFETIME(AOwnershipActor, TestNumber);
	DOREPLIFETIME(AOwnershipActor, MatColor);
}


// 1. �÷��̾� ĳ���Ͱ� �浹�ϸ� ������ �������� �����Ѵ�.
// 2. ���� ������ ��� Ŭ���̾�Ʈ���� �����ϰ� ������ �Ѵ�.
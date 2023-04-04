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

	// 서버 복제 on/off 함수
	SetReplicates(true);
	SetReplicateMovement(true);
}

// Called when the game starts or when spawned
void AOwnershipActor::BeginPlay()
{
	Super::BeginPlay();

	// 메시의 머티리얼을 다이나믹 머티리얼 인스턴스로 변경한다.
	UMaterialInterface* baseMaterial = MeshComponent->GetMaterial(0);
	if(baseMaterial != nullptr)
	{
		Mat = UMaterialInstanceDynamic::Create(baseMaterial, this);
		MeshComponent->SetMaterial(0, Mat);
	}

	// 만일 서버 액터라면
	// if(GetLocalRole() == ROLE_Authority)
	if(HasAuthority())
	{
		// 랜덤한 방향을 설정한다.
		MoveDirection = FMath::VRand();
		MoveDirection.Z = FMath::Abs(MoveDirection.Z);

		// 5초에 한 번씩 TestNumber의 변수를 변경한다.
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

	// 상태 정보를 출력한다.
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
		// 충돌한 대상이 플레이어라면
		if (OtherActor->IsA<ANetworkProjectCharacter>())
		{
			// 머티리얼 색상을 랜덤하게 변경한다.
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

// 지정된 범위 안에 플레이어가 있다면 가장 가까운 플레이어 캐릭터를 Owner로 설정한다.
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

	// 강사님 풀이
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
	// "MyColor" 파라미터의 값을 MatColor 값으로 변경한다.
	Mat->SetVectorParameterValue(FName("MyColor"), (FLinearColor)MatColor);
}

// 서버에 복제 등록하기 위한 함수
void AOwnershipActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AOwnershipActor, MoveDirection);
	DOREPLIFETIME(AOwnershipActor, TestNumber);
	DOREPLIFETIME(AOwnershipActor, MatColor);
}


// 1. 플레이어 캐릭터가 충돌하면 랜덤한 색상으로 변경한다.
// 2. 랜덤 색상은 모든 클라이언트에서 동일하게 보여야 한다.
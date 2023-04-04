// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetworkProjectCharacter.h"

#include "BattleGameMode.h"
#include "BattlePlayerController.h"
#include "BattlePlayerState.h"
#include "BattleSpectatorPawn.h"
#include "BulletActor.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "DrawDebugHelpers.h"
#include "PlayerAnimInstance.h"
#include "PlayerInfoWidget.h"
#include "ServerGameInstance.h"
#include "WeaponActor.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "Net/UnrealNetwork.h"
#include "UObject/ObjectPtr.h"


//////////////////////////////////////////////////////////////////////////
// ANetworkProjectCharacter

class ABattlePlayerState;

ANetworkProjectCharacter::ANetworkProjectCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	PlayerInfoUI = CreateDefaultSubobject<UWidgetComponent>(TEXT("Player Info UI"));
	PlayerInfoUI->SetupAttachment(GetMesh());

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void ANetworkProjectCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if(HasAuthority())
	{
		SetHealth(MaxHP);
	}

	InfoWidget = Cast<UPlayerInfoWidget>(PlayerInfoUI->GetWidget());

	PlayerAnim = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());

	GameInstance = Cast<UServerGameInstance>(GetGameInstance());

	if (GetController() != nullptr && GetController()->IsLocalController())
	{
		ServerSetName(GameInstance->SessionID.ToString());
	}

	/*FTimerHandle nameHandle;
	GetWorldTimerManager().SetTimer(nameHandle, FTimerDelegate::CreateLambda([this]()->void
		{
			InfoWidget->Text_Name->SetText(FText::FromString(MyName));
		}), 0.1f, false);*/

}

void ANetworkProjectCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	InfoWidget->Text_Name->SetText(FText::FromString(MyName));

	if (bIsDead)
	{
		return;
	}

	// 상태 정보를 출력한다.
	DrawDebugString(GetWorld(), GetActorLocation(), PrintInfo(), nullptr, FColor::White, 0.0f, true);

	if (HasAuthority())
	{
		Number++;
		RepNumber++;
	}

	// InfoWidget->ProgressBar_HP->SetPercent((float)(CurrentHP) / (float)MaxHP);
	InfoWidget->SetHealthBar(CurrentHP);

	/*auto animInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	animInstance->bIsDead = bIsDead;*/

	if (CurrentHP <= 0)
	{
		DieProcess();
	}
}

void ANetworkProjectCharacter::ServerSetName_Implementation(const FString& Name)
{
	MyName = Name;

	ABattlePlayerState* battlePlayerState = Cast<ABattlePlayerState>(GetPlayerState());

	if (battlePlayerState != nullptr)
	{
		battlePlayerState->SetPlayerName(Name);
	}
}

void ANetworkProjectCharacter::MulticastDamage_Implementation(int32 damage)
{
	if(CurrentHP > 0)
	{
		PlayAnimMontage(HitMontage);
	}
	else
	{
		if(PlayerAnim != nullptr)
		{
			PlayerAnim->bIsDead = true;
		}
	}
}

void ANetworkProjectCharacter::ServerDamage_Implementation(int32 damage)
{
	CurrentHP -= damage;
	/*if(CurrentHP > 0)
	{
		PlayAnimMontage(HitMontage);
	}
	else
	{
		auto animInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
		animInstance->bIsDead = bIsDead = true;
	}*/
	// MulticastDamage(damage);
	MulticastDamage(damage);
}

void ANetworkProjectCharacter::ClientFire_Implementation()
{
	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
	GetWorld()->SpawnActor<ABulletActor>(BulletFactory, SpawnLocation, GetActorRotation());

}

void ANetworkProjectCharacter::ClientDamage_Implementation(int32 damage)
{
	CurrentHP -= damage;
	ServerDamage(damage);
}

void ANetworkProjectCharacter::SetHealth(int32 Value)
{
	CurrentHP = FMath::Min(MaxHP, Value);
}

void ANetworkProjectCharacter::AddHealth(int32 Value)
{
	CurrentHP = FMath::Clamp(CurrentHP + Value, 0, MaxHP);
}

FString ANetworkProjectCharacter::PrintInfo()
{
	FString infoText = "";

#pragma region RoleInfo
	//FString myLocalRole = UEnum::GetValueAsString<ENetRole>(GetLocalRole());
	//FString myRemoteRole = UEnum::GetValueAsString<ENetRole>(GetRemoteRole());
	//FString myConnetion = GetNetConnection() != nullptr ? TEXT("Valid") : TEXT("Invalid");
	//// FString myOwner = GetNetOwner() != nullptr ? GetNetOwner()->GetName() : TEXT("No Owner");
	//FString myOwner = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");

	//infoText = FString::Printf(TEXT("Local Role: %s\nRemote Role: %s\nNet Connection: %s\nOwner: %s"), *myLocalRole, *myRemoteRole, *myConnetion, *myOwner);
#pragma endregion

#pragma region RepOrNot
	//infoText = FString::Printf(TEXT("Number: %d\n Replicated Number: %d"), Number, RepNumber);

#pragma endregion

#pragma region PlayerInfo
	/*APlayerController* playerController = Cast<APlayerController>(GetController());
	FString playerControllerString = playerController != nullptr ? FString(GetController()->GetName()) : FString("Has No Controller");
	FString gameModeString = GetWorld()->GetAuthGameMode() != nullptr ? FString("Has GameModeBase") : FString("Has No GameModeBase");
	FString gameStateString = GetWorld()->GetGameState() != nullptr ? FString("Has GameState") : FString("Has No GameState");
	FString playerStateString = GetPlayerState() != nullptr ? FString("Has PlayerState") : FString("Has No PlayerState");
	FString hudString;
	if(playerController != nullptr)
	{
		if(playerController->GetHUD() != nullptr)
		{
			hudString = playerController != nullptr ? playerController->GetHUD()->GetName() : FString("Has No HUD");
		}
	}

	infoText = FString::Printf(TEXT("%s\n%s\n%s\n%s\n%s"), *playerControllerString, *gameModeString, *gameStateString, *playerStateString, *hudString);*/
#pragma endregion

	FString playerStateName;
	if(GetPlayerState() != nullptr)
	{
		playerStateName = GetPlayerState()->GetPlayerName();
		// GetPlayerState()->GetScore();
		// GetPlayerState()->SetScore();
	}

	FString playerStateNames;
	if(GetWorld()->GetGameState() != nullptr)
	{
		for(TObjectPtr<APlayerState> ps : GetWorld()->GetGameState()->PlayerArray)
		{
			playerStateNames.Append(FString::Printf(TEXT("%s\n"), *ps->GetPlayerName()));
		}
	}

	infoText = FString::Printf(TEXT("Player State: %s\nGame State:\n%s"), *playerStateName, *playerStateNames);

	return infoText;
}

// 캐릭터 사망 시 처리 함수
void ANetworkProjectCharacter::DieProcess()
{
	bIsDead = true;

	// 조작을 하는 클라이언트에서만 실행한다.
	if (GetController() != nullptr) //  && GetController()->IsLocalController())
	{
		GetCharacterMovement()->DisableMovement();
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		bUseControllerRotationYaw = false;
		FollowCamera->PostProcessSettings.ColorSaturation = FVector4(0.f, 0.f, 0.f, 1);
		ReleaseWeapon();
	}

	if(HasAuthority())
	{
		FTimerHandle spectatorHandle;
		GetWorldTimerManager().SetTimer(spectatorHandle, FTimerDelegate::CreateLambda([this]()->void
			{
				// Cast<ABattlePlayerController>(GetController())->Respawn(this);
				ChangeSpectatorMode();
			}), 3.0f, false);
	}
}

// 관전자 모드로 변경하는 함수
void ANetworkProjectCharacter::ChangeSpectatorMode()
{
	ABattleGameMode* battleGameMode = Cast<ABattleGameMode>(GetWorld()->GetAuthGameMode());

	

	if(battleGameMode != nullptr)
	{
		// 게임 모드에 설정한 관전자 폰 클래스를 불러온다.
		TSubclassOf<ASpectatorPawn> spectatorPawn = battleGameMode->SpectatorClass;
		FActorSpawnParameters param;
		param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ABattleSpectatorPawn* spectator = GetWorld()->SpawnActor<ABattleSpectatorPawn>(spectatorPawn, GetActorLocation(), GetActorRotation(), param);

		if(spectator != nullptr)
		{
			spectator->OriginalPlayer = this;
			GetController()->Possess(spectator);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetworkProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetworkProjectCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetworkProjectCharacter::Look);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ANetworkProjectCharacter::Fire);

		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &ANetworkProjectCharacter::ReleaseWeapon);

	}
}

void ANetworkProjectCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetworkProjectCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// 총 발사
void ANetworkProjectCharacter::Fire()
{
	/*UE_LOG(LogTemp, Warning, TEXT("Query Fire"));
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("Query Fire"), true, FVector2D(1.2f));*/

	/*FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
	GetWorld()->SpawnActor<ABulletActor>(BulletFactory, SpawnLocation, GetActorRotation());*/

	// 만약 총을 가지고 있다면
	if(OwningWeapon != nullptr && !bFireDelay)
	{


		ServerFire();
	}
}

// 서버에 요청하는 함수
void ANetworkProjectCharacter::ServerFire_Implementation()
{
	/*UE_LOG(LogTemp, Warning, TEXT("Server Fire"));
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("Server Fire"), true, FVector2D(1.2f));*/

	// 만일 총알이 있으면 총알을 생성한다.
	if(CurrentAmmo > 0)
	{
		FActorSpawnParameters param;
		param.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
		ABulletActor* bullet = GetWorld()->SpawnActor<ABulletActor>(BulletFactory, OwningWeapon->MeshComponent->GetSocketLocation(FName("AmmoSocket")), OwningWeapon->MeshComponent->GetSocketRotation(FName("AmmoSocket")), param);
		bullet->SetOwner(this);
		// bullet->SetOwner(nullptr);
		bullet->AttackPower = OwningWeapon->DamagePower;
		CurrentAmmo--;
		OwningWeapon->Ammo--;
		bFireDelay = true;
		FTimerHandle FireDelayHandle;
		GetWorldTimerManager().SetTimer(FireDelayHandle, FTimerDelegate::CreateLambda([&]()->void{
			bFireDelay = false;
		}), OwningWeapon->ReloadTime, false);

		// ClientFire(500);
		MulticastFire(true);
	}
	// 만약 총알이 없으면
	else
	{
		MulticastFire(false);
	}
	
}

bool ANetworkProjectCharacter::ServerFire_Validate()
{
	return true;
}

// 서버로부터 전달되는 함수
void ANetworkProjectCharacter::MulticastFire_Implementation(bool bHasAmmo)
{
	/*UE_LOG(LogTemp, Warning, TEXT("Multicast Fire"));
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, FString("Multicast Fire"), true, FVector2D(1.2f));*/

	/*FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 100.f;
	GetWorld()->SpawnActor<ABulletActor>(BulletFactory, SpawnLocation, GetActorRotation());*/

	if(bHasAmmo)
	{
		if(FireMontage != nullptr)
		{
			PlayAnimMontage(FireMontage);
		}
	}
	else
	{
		if(NoAmmoMontage != nullptr)
		{
			PlayAnimMontage(NoAmmoMontage);
		}
	}

	
}

// 총 내려놓기
void ANetworkProjectCharacter::ReleaseWeapon()
{
	if(GetController() != nullptr && GetController()->IsLocalController() && OwningWeapon != nullptr)
	{
		OwningWeapon->ServerReleaseWeapon(this);
	}
}

void ANetworkProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// DOREPLIFETIME(ANetworkProjectCharacter, RepNumber);
	// DOREPLIFETIME_CONDITION(ANetworkProjectCharacter, RepNumber, COND_OwnerOnly);

	DOREPLIFETIME(ANetworkProjectCharacter, CurrentHP);
	DOREPLIFETIME(ANetworkProjectCharacter, CurrentAmmo);
	DOREPLIFETIME(ANetworkProjectCharacter, MyName);
	DOREPLIFETIME(ANetworkProjectCharacter, bFireDelay);
}

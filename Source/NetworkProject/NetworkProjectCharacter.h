// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "NetworkProjectCharacter.generated.h"


UCLASS(config=Game)
class ANetworkProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* PlayerInfoUI;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReleaseAction;

public:
	ANetworkProjectCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);



protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	virtual void Tick(float DeltaSeconds) override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(Server, Unreliable, WithValidation)
	void ServerFire();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFire(bool bHasAmmo);
	UFUNCTION(NetMulticast, Unreliable)
	void ClientFire();

	UFUNCTION(Server, Unreliable)
	void ServerDamage(int32 damage);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDamage(int32 damage);
	UFUNCTION(Client, Unreliable)
	void ClientDamage(int32 damage);

	UFUNCTION(Server, Unreliable)
	void ServerSetName(const FString& Name);

	UFUNCTION()
	void EndSession();

	UFUNCTION(Server, Unreliable)
	void ServerDestroyAllSessions();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDestroyAllSessions();

	UPROPERTY(EditAnywhere, Category="My Settings")
	TSubclassOf<class ABulletActor> BulletFactory;

	UPROPERTY(EditDefaultsOnly, Category="My Settings")
	class UAnimMontage* FireMontage;
	UPROPERTY(EditDefaultsOnly, Category="My Settings")
	class UAnimMontage* HitMontage;
	UPROPERTY(EditDefaultsOnly, Category="My Settings")
	class UAnimMontage* NoAmmoMontage;

	UPROPERTY(EditDefaultsOnly, Replicated, Category="My Settings")
	int32 CurrentHP;
	UPROPERTY(EditDefaultsOnly, Category="My Settings")
	int32 MaxHP = 100;
	UPROPERTY(EditDefaultsOnly, Replicated, Category="My Settings")
	int32 CurrentAmmo;
	UPROPERTY(VisibleDefaultsOnly, Category="My Settings")
	class UPlayerInfoWidget* InfoWidget;

	UPROPERTY()
	class AWeaponActor* OwningWeapon;

	UFUNCTION()
	void SetHealth(int32 Value);
	UFUNCTION()
	void AddHealth(int32 Value);
	UFUNCTION()
	int32 GetHealth() { return CurrentHP; };
	UFUNCTION()
	int32 GetAmmo() { return CurrentAmmo; };
	UFUNCTION()
	bool IsDead() { return bIsDead; };

	UFUNCTION()
	void Fire();
	UFUNCTION()
	void ReleaseWeapon();

	UFUNCTION()
	void DestoryMySession();

private:
	bool bIsDead = false;

	int32 Number;

	UPROPERTY(Replicated)
	int32 RepNumber;
	UPROPERTY(Replicated)
	FString MyName;
	UPROPERTY(Replicated)
	bool bFireDelay = false;

	class UPlayerAnimInstance* PlayerAnim;
	class UServerGameInstance* GameInstance;


	FString PrintInfo();

	void DieProcess();

	void ChangeSpectatorMode();
};


#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterType.h"
#include "Interfaces/PickupInterface.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class AItem;
class ASoul;
class UAnimMontage;
class USlashOverly;
        
UCLASS()
class SLASH_API ASlashCharacter : public ABaseCharacter, public IPickupInterface
{
	GENERATED_BODY()

public:
	ASlashCharacter();
	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	/* <AActor> */
	/* <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, const FVector& ImpactNormal, AActor* Hitter) override;
	/* <IHitInterface> */
	/* <ABaseCharacter> */
	virtual void SetOverlappingItem(AItem* Item) override;
	/* <ABaseCharacter> */
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	void CancelLock();

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bLocking = false;

	bool bIsCharging = false;
protected:
	virtual void BeginPlay() override;

	/* Callbacks for input */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Interact(const FInputActionValue& Value);
	void PressAttack(const FInputActionValue& Value);
	void Dodge(const FInputActionValue& Value);
	void ShiftRun(const FInputActionValue& Value);
	void LockEnemy(const FInputActionValue& Value);
	void ChangeLock(const FInputActionValue& Value);
	void ChargeStart(const FInputActionValue& Value);
	void ChargeRelease(const FInputActionValue& Value);
	void Execution(const FInputActionValue& Value);
	/* Callbacks for input */
	bool CanExecution();

	/* Combat */
	void EquipWeapon(AWeapon* Weapon);
	virtual void Attack() override;
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
    bool CanDisarm();
	bool CanArm();
	void DisArm();
	void Arm();
	void PlayEquipMontage(FName SectionName);
	virtual void Die_Implementation() override;
	bool HasStamina();
	bool HasEnoughStamina();
	bool IsOccupied();
	void OnChargeMaxed();
	void TriggerPerfectDodge();
	void ResetTimeDilation();

	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void SetMotionWarppingTarget();

	virtual void SetMotionWarppingTarget_Implementation();

	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void SetExecutionMotionWarppingTarget();

	virtual void SetExecutionMotionWarppingTarget_Implementation();
	
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToTough();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UPROPERTY(BlueprintReadOnly, Category = "Input")
	bool isRun = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsPerfectDodge = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsInvincible = false;

	FTimerHandle PerfectDodgeTimer;
	/* Combat */

	//
	//2.27
	//
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	EAttackState AttackState = EAttackState::EAS_ComboAttackSeq01;

	//–Ó¡¶
	float ChargeStartTime = 0.f;

	UPROPERTY(EditAnywhere)
	float MaxChargeTime = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ChargeMontage;
	FTimerHandle ChargeMaxTimer;

private:
	bool IsUnoccupied();
	/* UI */
	void InitializeSlashOverlay();
	void SetHUDHealth();
	/* UI */

	/* Input */
	UPROPERTY(EditAnywhere, Category = "Input")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ShiftRunAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LockAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChangeLockAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ChargeAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* ExecutionAction;
	/* Input */

	/* Character components */
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;
	/* Character components */

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	/*
	* Animation Montages
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipMontage;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY()
	USlashOverly* SlashOverly;

public:
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }
};

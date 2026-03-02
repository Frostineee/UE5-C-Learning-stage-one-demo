#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "InputAction.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/CombatComponent.h"
#include "Components/WidgetComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"	
#include "Animation/AnimMontage.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverly.h"
#include "Interfaces/PickupInterface.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Enemy/Enemy.h"
#include "Kismet/GameplayStatics.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 300.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
}

void ASlashCharacter::Tick(float DeltaTime)
{
	if (Attributes && SlashOverly)
	{
		Attributes->RegenStamina(DeltaTime);
		SlashOverly->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);

	Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);

	Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ASlashCharacter::Jump);

	Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ASlashCharacter::Interact);

	Input->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::PressAttack);

	Input->BindAction(DodgeAction, ETriggerEvent::Started, this, &ASlashCharacter::Dodge);

	Input->BindAction(ShiftRunAction, ETriggerEvent::Started, this, &ASlashCharacter::ShiftRun);

	Input->BindAction(LockAction, ETriggerEvent::Started, this, &ASlashCharacter::LockEnemy);

	Input->BindAction(ChangeLockAction, ETriggerEvent::Started, this, &ASlashCharacter::ChangeLock);

	Input->BindAction(ChargeAction, ETriggerEvent::Started, this, &ASlashCharacter::ChargeStart);

	Input->BindAction(ChargeAction, ETriggerEvent::Completed, this, &ASlashCharacter::ChargeRelease);

	Input->BindAction(ExecutionAction, ETriggerEvent::Started, this, &ASlashCharacter::Execution);
}

void ASlashCharacter::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (ActionState == EActionState::EAS_Executing) return 0.f;

	if (bIsPerfectDodge)
	{
		TriggerPerfectDodge();
		return 0.f;
	}

	if (bIsInvincible)
	{
		return 0.f;
	}

	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, const FVector& ImpactNormal, AActor* Hitter)
{
	if (ActionState == EActionState::EAS_Executing) return;

	if (bIsPerfectDodge || bIsInvincible) return;

	Super::GetHit_Implementation(ImpactPoint, ImpactNormal, Hitter);

	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && SlashOverly)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverly->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverly)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverly->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (!InputMapping.IsNull())
			{
				Subsystem->AddMappingContext(InputMapping.LoadSynchronous(), 0);
			}
		}
	}

	Tags.Add(FName("EngageableTarget"));

	InitializeSlashOverlay();
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (MovementVector.X != 0)
		{
			const FRotator ControlRotation = GetControlRotation();
			const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			AddMovementInput(Direction, MovementVector.X);
		}

		if (MovementVector.Y != 0)
		{
			const FRotator ControlRotation = GetControlRotation();
			const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

			const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(Direction, MovementVector.Y);
		}
	}
	
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		if (LookAxisVector.X != 0)
		{
			AddControllerYawInput(LookAxisVector.X);
		}

		if (LookAxisVector.Y != 0)
		{
			AddControllerPitchInput(-LookAxisVector.Y);
		}
	}
}

void ASlashCharacter::Interact(const FInputActionValue& Value)
{
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	if (OverlappingWeapon)
	{
		if (EquippedWeapon)
		{
			EquippedWeapon->Destroy();
			
		}
		EquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanDisarm())
		{
			DisArm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::PressAttack(const FInputActionValue& Value)
{		
	Attack();
	SetMotionWarppingTarget();
}

void ASlashCharacter::Dodge(const FInputActionValue& Value)
{
	if (IsOccupied() || !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && SlashOverly)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		SlashOverly->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::ShiftRun(const FInputActionValue& Value)
{
	isRun = !isRun;
	if (isRun)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = 200.f;
	}
}

void ASlashCharacter::LockEnemy(const FInputActionValue& Value)
{
	if (!bLocking)
	{
		Combat->LockingEnemy();
	}
	else
	{
		CancelLock();
	}

}

void ASlashCharacter::CancelLock()
{
	if (bLocking)
	{
		if (const AEnemy* Enemy = Cast<AEnemy>(Combat->LockActor))
		{
			Enemy->LockWidget->SetVisibility(false);
		}
		Combat->TargetLockActors.Empty();
		Combat->TargetIndex = 0;
		//没做混合空间2D，暂时不修改
		//GetCharacterMovement()->bUseControllerDesiredRotation = false;
		//GetCharacterMovement()->bOrientRotationToMovement = true;
		bLocking = false;
		CombatTarget = nullptr;
	}
	
}

void ASlashCharacter::ChangeLock(const FInputActionValue& Value)
{
	if (bLocking)
	{
		Combat->ChangeLockTarget();
	}
}

void ASlashCharacter::ChargeStart(const FInputActionValue& Value)
{
	if (CanAttack())
	{
		bIsCharging = true;

		ChargeStartTime = GetWorld()->GetTimeSeconds();

		ActionState = EActionState::EAS_Attacking;

		PlayMontageSection(ChargeMontage, FName("ChargeStart"));

		if (EquippedWeapon)
		{
			EquippedWeapon->StartCharge();
		}

		GetWorldTimerManager().SetTimer(ChargeMaxTimer, this, &ASlashCharacter::OnChargeMaxed, MaxChargeTime, false);
	}
}

void ASlashCharacter::ChargeRelease(const FInputActionValue& Value)
{
	if (bIsCharging)
	{
		bIsCharging = false;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (!AnimInstance) return;

		float ChargeDuration = GetWorld()->GetTimeSeconds() - ChargeStartTime;
		AnimInstance->Montage_Stop(0.1f, ChargeMontage);

		if (EquippedWeapon)
		{
			EquippedWeapon->EndCharge();
		}

		GetWorldTimerManager().ClearTimer(ChargeMaxTimer);

		if (ChargeDuration >= MaxChargeTime)
		{
			if (bLocking)
			{
				CombatTarget = Combat->LockActor;
			}
			else
			{
				CombatTarget = Combat->NoLockClosestEnemy();
			}

			SetMotionWarppingTarget();
			PlayMontageSection(ChargeMontage, FName("ChargeComplete"));
		}
		else
		{
			ActionState = EActionState::EAS_Unoccupied;
			Attack();
			SetMotionWarppingTarget();
		}		
	}
}

void ASlashCharacter::Execution(const FInputActionValue& Value)
{
	if (!CanExecution()) return;
	ActionState = EActionState::EAS_Executing;
	AEnemy* ExecutionTarget = Cast<AEnemy>(CombatTarget);

	SetExecutionCollision();

	EquippedWeapon->SetDamage(99999.f);

	SetExecutionMotionWarppingTarget();
	PlayMontageSection(ExecutingMontage);
	ExecutionTarget->PlayBeingExecutedMontage();
}

bool ASlashCharacter::CanExecution()
{
	if (!CanAttack()) return false;
	if (!CombatTarget) return false;

	AEnemy* ExecutionTarget = Cast<AEnemy>(CombatTarget);
	if (ExecutionTarget)
	{
		if (!ExecutionTarget->GetAttributeComponent()->IsAlive()) return false;
		if (ExecutionTarget->GetAttributeComponent()->GetHealthPercent() > 0.1f) return false;
		if (!ExecutionTarget->BeingExecutedMontage) return false;
	}

	return true;
}

void ASlashCharacter::OnChargeMaxed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->ChargeMaxedFeedBack();
	}
}

void ASlashCharacter::TriggerPerfectDodge()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.2f);
	GetWorldTimerManager().SetTimer(PerfectDodgeTimer, this, &ASlashCharacter::ResetTimeDilation, 0.1f, false);
	if (Attributes)
	{
		Attributes->RegenStamina(100.f);
	}
}

void ASlashCharacter::ResetTimeDilation()
{
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("hand_r_Socket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	EquippedWeapon = Weapon;
	OverlappingItem = nullptr;
}

void ASlashCharacter::Attack()
{
	if (CanAttack())
	{
		if (bLocking)
		{
			CombatTarget = Combat->LockActor;
		}
		else
		{
			CombatTarget = Combat->NoLockClosestEnemy();
		}

		Super::Attack();

		switch (AttackState)
		{
		case EAttackState::EAS_ComboAttackSeq01 :
			PlayMontageSection(AttackMontage, AttackMontageSections[0]);
			break;
		case EAttackState::EAS_ComboAttackSeq02 :
			PlayMontageSection(AttackMontage, AttackMontageSections[1]);
			break;
		case EAttackState::EAS_ComboAttackSeq03 :
			PlayMontageSection(AttackMontage, AttackMontageSections[2]);
			break;
		case EAttackState::EAS_ComboAttackSeq04 :
			PlayMontageSection(AttackMontage, AttackMontageSections[3]);
			break;
		}
		ActionState = EActionState::EAS_Attacking;
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_EquippedOneHandedWeapon;
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied&&
		CharacterState == ECharacterState::ECS_Unequipped&&
		EquippedWeapon;
}

void ASlashCharacter::DisArm()
{
	PlayEquipMontage(FName("Unequip"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Equip"));
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ASlashCharacter::SetMotionWarppingTarget_Implementation()
{
}

void ASlashCharacter::SetExecutionMotionWarppingTarget_Implementation()
{
}

void ASlashCharacter::AttachWeaponToTough()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("thigh_l_Socket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("hand_r_Socket"));
	}
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverly = SlashHUD->GetSlashOverlay();
			if (SlashOverly && Attributes)
			{
				SlashOverly->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverly->SetStaminaBarPercent(1.0f);
				SlashOverly->SetGold(0);
				SlashOverly->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverly && Attributes)
	{
		SlashOverly->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}

void ASlashCharacter::PlayEquipMontage(FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();
	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
}

bool ASlashCharacter::HasStamina()
{
	return Attributes && Attributes->GetStaminaPercent() > 0.f;
}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}



#include "Components/CombatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/SlashCharacter.h"
#include "Enemy/Enemy.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	CameraForwardLength = 500.f;
	AttackCheckRange = 300.f;
	MinDistance = 99999.f;
	TargetIndex = 0;
	LockCheckRange = 700.f;
	ChangeLockCheckRange = 100.f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetSlashCharacter()->bLocking && LockActor)
	{
		if (LockActor->ActorHasTag("Dead"))
		{
			GetSlashCharacter()->CancelLock();
		}
		else if (TargetLockActors.Num() > 0)
		{
			FocusOnTarget(TargetLockActors[TargetIndex]);
		}
	}
}

AActor* UCombatComponent::NoLockClosestEnemy()
{
	const FVector StartLocation = GetSlashCharacter()->GetActorLocation();
	const FVector EndLocation = GetSlashCharacter()->ViewCamera->GetComponentLocation() + 
		                        GetSlashCharacter()->ViewCamera->GetForwardVector() * CameraForwardLength;
	TArray<FHitResult> HitResults;
	TargetActors.Empty();
	if (UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		StartLocation,
		EndLocation,
		AttackCheckRange,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::Type::ForDuration,
		HitResults,
		true
	))
	{
		for (auto& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor->ActorHasTag("Enemy"))
			{
				TargetActors.AddUnique(HitActor);
			}
		}

		for (auto& Target : TargetActors)
		{
			const float Distance = Target->GetDistanceTo(GetSlashCharacter());
			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				ClosestActor = Target;
			}
		}
		MinDistance = 99999.f;
		return ClosestActor;
	}
	return nullptr;
}

bool UCombatComponent::CompareActorFloat(const AActor& ActorA, const AActor& ActorB)
{
	return ActorA.GetDistanceTo(GetSlashCharacter()) < ActorB.GetDistanceTo(GetSlashCharacter());
}

AActor* UCombatComponent::LockingEnemy()
{
	const FVector Location = GetSlashCharacter()->GetActorLocation();
	TArray<FHitResult> HitResults;
	TargetLockActors.Empty();
	if (UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Location,
		Location,
		LockCheckRange,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		TArray<AActor*>(),
		EDrawDebugTrace::Type::ForDuration,
		HitResults,
		true
	))
	{
		for (auto& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor->ActorHasTag("Enemy"))
			{
				TargetLockActors.AddUnique(HitActor);
			}
		}

		TargetLockActors.StableSort(
			[this](const AActor& A, const AActor& B)
			{
				return CompareActorFloat(A, B);
			});

		GetSlashCharacter()->bLocking = true;
		//GetSlashCharacter()->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		//GetSlashCharacter()->GetCharacterMovement()->bOrientRotationToMovement = false;
		LockActor = TargetLockActors[TargetIndex];
		Cast<AEnemy>(LockActor)->LockWidget->SetVisibility(true);
		return LockActor;
	}

	GetSlashCharacter()->bLocking = false;
	//GetSlashCharacter()->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	//GetSlashCharacter()->GetCharacterMovement()->bOrientRotationToMovement = true;
	return nullptr;
}

void UCombatComponent::FocusOnTarget(const AActor* Target) const
{
	const FVector StartLocation = GetSlashCharacter()->GetActorLocation();
	const FVector TargetLocation = Target->GetActorLocation();

	const FRotator StartToTargetRotator = UKismetMathLibrary::FindLookAtRotation(StartLocation, TargetLocation);
	const FRotator CurrentRotator = GetSlashCharacter()->GetController()->GetControlRotation();
	const FRotator TargetRotator(FRotator(CurrentRotator.Pitch, StartToTargetRotator.Yaw, CurrentRotator.Roll));

	const FRotator SmoothRotator = FMath::RInterpTo(CurrentRotator, TargetRotator, GetWorld()->GetDeltaSeconds(), InterpSpeed);
	GetSlashCharacter()->GetController()->SetControlRotation(SmoothRotator);
}

void UCombatComponent::ChangeLockTarget()
{
	const FVector Location = GetSlashCharacter()->GetActorLocation();
	TArray<FHitResult> HitResults;
	if (UKismetSystemLibrary::SphereTraceMulti(
		GetWorld(),
		Location,
		Location,
		LockCheckRange,
		UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel1),
		false,
		TargetLockActors,
		EDrawDebugTrace::Type::ForDuration,
		HitResults,
		true
	))
	{
		for (auto& HitResult : HitResults)
		{
			AActor* HitActor = HitResult.GetActor();
			if (HitActor->ActorHasTag("Enemy"))
			{
				TargetLockActors.AddUnique(HitActor);
			}
		}

		TargetLockActors.StableSort(
			[this](const AActor& A, const AActor& B) ->bool
			{
				return CompareActorFloat(A, B);
			});

		Cast<AEnemy>(LockActor)->LockWidget->SetVisibility(false);
		TargetIndex = 0;
		LockActor = TargetLockActors[0];
		Cast<AEnemy>(LockActor)->LockWidget->SetVisibility(true);
		return;
	}

	if (TargetLockActors.Num() > 0)
	{
		TargetIndex = (TargetIndex + 1) % TargetLockActors.Num();
		Cast<AEnemy>(LockActor)->LockWidget->SetVisibility(false);
		LockActor = TargetLockActors[TargetIndex];
		Cast<AEnemy>(LockActor)->LockWidget->SetVisibility(true);
	}
}

ASlashCharacter* UCombatComponent::GetSlashCharacter() const
{
	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(GetOwner()))
	{
		return SlashCharacter;
	}

	return nullptr;
}


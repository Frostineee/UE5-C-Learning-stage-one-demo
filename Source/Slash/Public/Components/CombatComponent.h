#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class ASlashCharacter;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SLASH_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//ÎÞË÷µÐ¹¥»÷Îü¸½
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CameraForwardLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackCheckRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance;

	UPROPERTY()
	TArray<AActor*> TargetActors;

	UPROPERTY()
	TObjectPtr<AActor> ClosestActor;

	UFUNCTION(BlueprintCallable)
	AActor* NoLockClosestEnemy();
	//ÎÞË÷µÐ¹¥»÷Îü¸½ end


	//Ë÷µÐ
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LockCheckRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ChangeLockCheckRange;

	int32 TargetIndex;

	UPROPERTY()
	TArray<AActor*> TargetLockActors;

	TObjectPtr<AActor> LockActor;

	bool CompareActorFloat(const AActor& ActorA, const AActor& ActorB);

	AActor* LockingEnemy();
	void FocusOnTarget(const AActor* Target) const;
	void ChangeLockTarget();

	UPROPERTY(EditAnywhere)
	float InterpSpeed = 15.f;
	//Ë÷µÐ end
	
private:
	ASlashCharacter* GetSlashCharacter() const;
		
};

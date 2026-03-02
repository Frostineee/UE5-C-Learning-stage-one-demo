#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces//HitInterface.h"
#include "BreakableActor.generated.h"

class UGeometryCollectionComponent;

UCLASS()
class SLASH_API ABreakableActor : public AActor, public IHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();
	/* <AActor> */
	virtual void Tick(float DeltaTime) override;
	/* <AActor> */

	/* <IHitInterface> */
	// ÊÜ»÷SpawnTreasure
	virtual void GetHit_Implementation(const FVector& ImpactPoint, const FVector& ImpactNormal, AActor* Hitter) override;
	/* <IHitInterface> */

	UFUNCTION(BlueprintCallable)
	void SpawnTreasure();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UGeometryCollectionComponent* GeometryCollection;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UCapsuleComponent* Capsule;

private:
	UPROPERTY(EditAnywhere, Category = "Breakalbe Properties")
	TArray<TSubclassOf<class ATreasure>> TreasureClasses;

	bool bBroken = false;
}; 

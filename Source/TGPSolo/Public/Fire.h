// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Fire.generated.h"

class USphereComponent;

UCLASS()
class TGPSOLO_API AFire : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFire();

	UPROPERTY(VisibleAnywhere, Category = "Components")
		USphereComponent* SphereComp;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float FireDamage = 0.1f;

	void NotifyActorBeginOverlap(AActor* OtherActor);
	void NotifyActorEndOverlap(AActor * OtherActor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Fire")
		void OnActivate(AActor* collidedWith);

	UFUNCTION(BlueprintCallable, Category = "Damage")
		float GetDamageValue();

	UPROPERTY(EditAnywhere, Category = "FX")
		float fireParticleSize;

	UPROPERTY(EditAnywhere, Category = "FX")
		class UParticleSystem* FireParticles;

	UPROPERTY(EditAnywhere, Category = "FX")
		class USoundBase* FireSound;

	bool onFire = false;
	AActor *character;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};

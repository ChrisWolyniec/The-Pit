// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine.h"
#include "GameFramework/Actor.h"
#include "Barrel.generated.h"

UCLASS()
class TGPSOLO_API ABarrel : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "FX")
		class UParticleSystem* ExplosionParticles;

	UPROPERTY(EditAnywhere, Category = "FX")
		class USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, Category = "Grenade")
		float Radius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Damage")
		float damage = -200.0f;
	
public:	
	// Sets default values for this actor's properties
	ABarrel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Ignite();

	void OnDetonate();

	bool onFire = false;

	UPROPERTY(EditDefaultsOnly, Category = Fire)
		TSubclassOf<class AFire> FireClass;

	
	
};

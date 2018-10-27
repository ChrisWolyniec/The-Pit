// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "Target.generated.h"

UCLASS()
class TGPSOLO_API ATarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void DestroyTarget();

	UPROPERTY(EditAnywhere, Category = "Type")
		bool Enemy = true;
	
	
};

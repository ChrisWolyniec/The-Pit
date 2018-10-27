// Fill out your copyright notice in the Description page of Project Settings.

#include "Target.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"
#include "TGPSoloCharacter.h"

//Purely for debug
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
// End debug



// Sets default values
ATarget::ATarget()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ATarget::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATarget::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATarget::DestroyTarget()
{
	TArray<FHitResult> HitObjects;
	FVector StartTrace = GetActorLocation();
	FVector EndTrace = StartTrace;
	EndTrace.Z += 300.0f;

	FCollisionShape CollisionShape;
	CollisionShape.ShapeType = ECollisionShape::Sphere;
	CollisionShape.SetSphere(5.0f);


	if (GetWorld()->SweepMultiByChannel(HitObjects, StartTrace, EndTrace, FQuat::FQuat(), ECC_WorldStatic, CollisionShape))
	{
		for (auto Object = HitObjects.CreateIterator(); Object; Object++)
		{
			ADestructibleActor* destructibleActor = Cast<ADestructibleActor>((*Object).GetActor());

			if (destructibleActor)
			{
				destructibleActor->GetDestructibleComponent()->ApplyRadiusDamage(10.0f, Object->ImpactPoint, 5.0f, 3000.0f, false);
			}
		}
	}

	ACharacter* character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	ATGPSoloCharacter* tgpCharacter = Cast<ATGPSoloCharacter>(character);

	if (Enemy == true)
	{
		tgpCharacter->UpdateEnemiesKilled();
	}
	else
	{
		tgpCharacter->UpdateHostagesKilled();
	}

	Destroy();
}

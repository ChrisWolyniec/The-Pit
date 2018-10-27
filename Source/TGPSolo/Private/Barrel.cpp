// Fill out your copyright notice in the Description page of Project Settings.

#include "Barrel.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TGPSoloCharacter.h"
#include "Target.h"
#include "Fire.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"



//Purely for debug
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
// End debug



// Sets default values
ABarrel::ABarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABarrel::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABarrel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABarrel::Ignite()
{
	UWorld* const World = GetWorld();
	if (!onFire)
	{
		const FRotator SpawnRotation = this->GetActorRotation();
		// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
		FVector SpawnLocation = this->GetActorLocation();
		SpawnLocation.Z += 80;

		//Set Spawn Collision Handling Override
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		// spawn the projectile at the muzzle
		World->SpawnActor<AFire>(FireClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
		onFire = true;
	}
	else if (onFire)
	{
		OnDetonate();
	}
}

void ABarrel::OnDetonate()
{
	UParticleSystemComponent* Explosion = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticles, GetActorTransform());
	Explosion->SetRelativeScale3D(FVector(4.0f));

	UGameplayStatics::PlaySoundAtLocation(this, ExplosionSound, GetActorLocation());

	TArray<FHitResult> HitObjects;

	FVector StartTrace = GetActorLocation();
	FVector EndTrace = StartTrace;
	EndTrace.Z += 300.0f;

	FCollisionShape CollisionShape;
	CollisionShape.ShapeType = ECollisionShape::Sphere;
	CollisionShape.SetSphere(Radius);


	if (GetWorld()->SweepMultiByChannel(HitObjects, StartTrace, EndTrace, FQuat::FQuat(), ECC_WorldStatic, CollisionShape))
	{
		for (auto Object = HitObjects.CreateIterator(); Object; Object++)
		{
			UStaticMeshComponent* staticMesh = Cast<UStaticMeshComponent>((*Object).Actor->GetRootComponent());
			ADestructibleActor* destructibleActor = Cast<ADestructibleActor>((*Object).GetActor());
			ATGPSoloCharacter* character = Cast<ATGPSoloCharacter>((*Object).GetActor());
			ATarget* target = Cast<ATarget>((*Object).GetActor());
			AFire* fire = Cast<AFire>((*Object).GetActor());

			if (target)
			{
				target->DestroyTarget();
			}
			else if (staticMesh)
			{
				staticMesh->AddRadialImpulse(GetActorLocation(), 1000.0f, 5000.0f, ERadialImpulseFalloff::RIF_Linear, true);
			}
			else if (destructibleActor)
			{
				destructibleActor->GetDestructibleComponent()->ApplyRadiusDamage(10.0f, Object->ImpactPoint, 500.0f, 3000.0f, false);
			}
			else if (character)
			{
				//FVector charLocation = 
				float distance = sqrt(FMath::Abs((pow(character->GetActorLocation().X - StartTrace.X, 2)) + (pow(character->GetActorLocation().Y - StartTrace.Y, 2)) + (pow(character->GetActorLocation().Z - StartTrace.Z, 2))));
				FString dist = FString::SanitizeFloat(distance);
				float damagePercent = 1 - (distance / Radius);
				float damageDealt = damage * damagePercent;
				character->UpdateHealth(damageDealt);
			}
			else if (fire)
			{
				fire->DestroyFire();
			}
		}
	}

	Destroy();
}

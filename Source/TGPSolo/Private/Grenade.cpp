// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "TGPSoloCharacter.h"
#include "Target.h"
#include "Barrel.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "WorldCollision.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"


// Sets default values
AGrenade::AGrenade()
{
 	/* Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.*/
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	/*CollisionComp->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);*/		// set up a notification for when this component hits something blocking

																					/* Players can't walk on it*/
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->Bounciness = 0.1;
	ProjectileMovement->Friction = 0.4f;

	// Die after 5 seconds by default
	InitialLifeSpan = duration;

}

//void AGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
//{
//	
//}




// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	
}



// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	timeSinceThrown += DeltaTime;

	if (timeSinceThrown > duration)
	{
		OnDetonate();
	}

}


void AGrenade::OnDetonate()
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

	int arraySize = HitObjects.Num();

	if (arraySize < 22)
	{
		if (GetWorld()->SweepMultiByChannel(HitObjects, StartTrace, EndTrace, FQuat::FQuat(), ECC_WorldStatic, CollisionShape))
		{
			for (auto Object = HitObjects.CreateIterator(); Object; Object++)
			{
				//UStaticMeshComponent* staticMesh = Cast<UStaticMeshComponent>((*Object).Actor->GetRootComponent());
				ADestructibleActor* destructibleActor = Cast<ADestructibleActor>((*Object).GetActor());
				ATGPSoloCharacter* character = Cast<ATGPSoloCharacter>((*Object).GetActor());
				ATarget* target = Cast<ATarget>((*Object).GetActor());
				ABarrel* barrel = Cast<ABarrel>((*Object).GetActor());

				if (target)
				{
					target->DestroyTarget();
				}
				/*else if (staticMesh)
				{
					staticMesh->AddRadialImpulse(GetActorLocation(), 1000.0f, 5000.0f, ERadialImpulseFalloff::RIF_Linear, true);
				}*/
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
				else if (barrel)
				{
					barrel->OnDetonate();
				}
			}
		}
	}
	Destroy();
}

void AGrenade::SetDuration(float heldTime)
{
		timeSinceThrown = heldTime;
}
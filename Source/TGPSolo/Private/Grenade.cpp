// Fill out your copyright notice in the Description page of Project Settings.

#include "Grenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	//PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AGrenade::OnHit);		// set up a notification for when this component hits something blocking

																					// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;

}

void AGrenade::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != NULL) && (OtherActor != this) && (OtherComp != NULL) && OtherComp->IsSimulatingPhysics()) //May need to delete last two sections? TODELETE
	{
		OnDetonate();
	}
}




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

	if (timeSinceThrown > 5.0f)
	{
		OnDetonate();
	}

}


void AGrenade::OnDetonate()
{
	UParticleSystemComponent* Explosion = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionParticles, GetActorTransform(), true);
	Explosion->SetRelativeScale3D(FVector(4.0f));
}
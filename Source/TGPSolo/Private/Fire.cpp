// Fill out your copyright notice in the Description page of Project Settings.

#include "Fire.h"
#include "Components/SphereComponent.h"
#include "../Public/Fire.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AFire::AFire()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
}

void AFire::NotifyActorBeginOverlap(AActor * OtherActor)
{
	onFire = true;
	character = OtherActor;
}

void AFire::NotifyActorEndOverlap(AActor * OtherActor)
{
	onFire = false;
}


// Called when the game starts or when spawned
void AFire::BeginPlay()
{
	Super::BeginPlay();
	Fire = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FireParticles, GetActorTransform(), false);
	Fire->SetRelativeScale3D(FVector(fireParticleSize));

	UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
}

// Called every frame
void AFire::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (onFire)
	{
		OnActivate(character);
	}
}

float AFire::GetDamageValue()
{
	return FireDamage;
}

void AFire::DestroyFire()
{
	Fire->SetRelativeScale3D(FVector(0));
	Destroy();
}



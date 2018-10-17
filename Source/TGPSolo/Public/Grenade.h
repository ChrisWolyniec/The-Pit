
// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EngineMinimal.h"
#include "GameFramework/Actor.h"
#include "Grenade.generated.h"

UCLASS(config = Game)
class AGrenade : public AActor
{
	GENERATED_BODY()

		/** Sphere collision component */
		UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		class USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(EditAnywhere, Category = "FX")
		class UParticleSystem* ExplosionParticles;

	UPROPERTY(EditAnywhere, Category = "FX")
		class USoundBase* ExplosionSound;

	UPROPERTY(EditAnywhere, Category = "Grenade")
		float Radius = 500.0f;

public:
	AGrenade();

	virtual void BeginPlay() override;
	void Tick(float DeltaTime);

	float timeSinceThrown = 0.0f;

	UFUNCTION()
	void OnDetonate();

	/** called when projectile hits something */
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/** Returns CollisionComp subobject **/
	FORCEINLINE class USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	FORCEINLINE class UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }
};


// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TGPSoloCharacter.h"
#include "TGPSoloProjectile.h"
#include "Grenade.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h" // for FXRMotionControllerBase::RightHandSourceId



#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "WorldCollision.h"
#include "DestructibleActor.h"
#include "DestructibleComponent.h"


//Purely for debug
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
// End debug

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// ATGPSoloCharacter

ATGPSoloCharacter::ATGPSoloCharacter()
{
	CurrentHealth = MaxHealth;

	CurrentAmmoLoaded = 0;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 0.0f, 10.0f);

	// Note: The ProjectileClass and the skeletal mesh/anim blueprints for Mesh1P, FP_Gun, and VR_Gun 
	// are set in the derived blueprint asset named MyCharacter to avoid direct content references in C++.

	// Create VR Controllers.
	R_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("R_MotionController"));
	R_MotionController->MotionSource = FXRMotionControllerBase::RightHandSourceId;
	R_MotionController->SetupAttachment(RootComponent);
	L_MotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("L_MotionController"));
	L_MotionController->SetupAttachment(RootComponent);

	// Create a gun and attach it to the right-hand VR controller.
	// Create a gun mesh component
	VR_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("VR_Gun"));
	VR_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	VR_Gun->bCastDynamicShadow = false;
	VR_Gun->CastShadow = false;
	VR_Gun->SetupAttachment(R_MotionController);
	VR_Gun->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

	VR_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("VR_MuzzleLocation"));
	VR_MuzzleLocation->SetupAttachment(VR_Gun);
	VR_MuzzleLocation->SetRelativeLocation(FVector(0.000004, 53.999992, 10.000000));
	VR_MuzzleLocation->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));		// Counteract the rotation of the VR gun model.

	// Uncomment the following line to turn motion controllers on by default:
	//bUsingMotionControllers = true;
}

void ATGPSoloCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));

	// Show or hide the two versions of the gun based on whether or not we're using motion controllers.
	if (bUsingMotionControllers)
	{
		VR_Gun->SetHiddenInGame(false, true);
		Mesh1P->SetHiddenInGame(true, true);
	}
	else
	{
		VR_Gun->SetHiddenInGame(true, true);
		Mesh1P->SetHiddenInGame(false, true);
	}
}

void ATGPSoloCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	deltaTime = DeltaTime;
	timeSinceLastShot += DeltaTime;
	sinceLastThrow += DeltaTime;
	if (timeSinceLastShot > 0.2)
	{
		canFire = true;
	}
	else
	{
		canFire = false;
	}

	if (grenadeHeld)
	{
		heldTime += DeltaTime;
		if (heldTime > 3)
		{
			OnThrowEnd();
			justThrown = true;
		}
	}


	FString enemiesKilledOutput = FString::FromInt(enemiesKilled);
	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, TEXT("Enemies Killed: ") + enemiesKilledOutput);

	FString hostagesKilledOutput = FString::FromInt(hostagesKilled);
	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, TEXT("Hostages Killed: ") + hostagesKilledOutput);

	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, TEXT("    "));

	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, TEXT("Health: ") + FString::FromInt(CurrentHealth));
	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, TEXT("Grenades: ") + FString::FromInt(CurrentGrenades));
	GEngine->AddOnScreenDebugMessage(-1, deltaTime, FColor::Red, TEXT("Ammo: ") + FString::FromInt(CurrentAmmoLoaded) + TEXT(" / ") + FString::FromInt(CurrentAmmo));

}

//////////////////////////////////////////////////////////////////////////
// Input

void ATGPSoloCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATGPSoloCharacter::OnFireSemi);
	PlayerInputComponent->BindAxis("FireAuto", this, &ATGPSoloCharacter::OnFireAuto);
	PlayerInputComponent->BindAction("ToggleFireRate", IE_Pressed, this, &ATGPSoloCharacter::ToggleFireRate);

	// Bind grenade events
	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &ATGPSoloCharacter::OnThrowStart);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &ATGPSoloCharacter::OnThrowEnd);
	PlayerInputComponent->BindAction("ToggleGrenades", IE_Released, this, &ATGPSoloCharacter::ToggleGrenade);


	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ATGPSoloCharacter::OnResetVR);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ATGPSoloCharacter::Reload);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &ATGPSoloCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATGPSoloCharacter::MoveRight);

	// Bind sprint events
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ATGPSoloCharacter::SprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ATGPSoloCharacter::SprintEnd);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ATGPSoloCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ATGPSoloCharacter::LookUpAtRate);



}

void ATGPSoloCharacter::OnFireAuto(float value)
{
	if (value >= 1 && FullAuto)
	{
		OnFire();
	}
}

void ATGPSoloCharacter::OnFireSemi()
{
	if (!FullAuto)
	{
		OnFire();
	}
}

void ATGPSoloCharacter::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL && CurrentAmmoLoaded > 0 && canFire)
	{
		timeSinceLastShot = 0.0f;
		CurrentAmmoLoaded--;
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			if (bUsingMotionControllers)
			{
				const FRotator SpawnRotation = VR_MuzzleLocation->GetComponentRotation();
				const FVector SpawnLocation = VR_MuzzleLocation->GetComponentLocation();
				World->SpawnActor<ATGPSoloProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
			else
			{
				const FRotator SpawnRotation = GetControlRotation();
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				//Set Spawn Collision Handling Override
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

				// spawn the projectile at the muzzle
				World->SpawnActor<ATGPSoloProjectile>(ProjectileClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			}
		}

		// try and play the sound if specified
		if (FireSound != NULL)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

		// try and play a firing animation if specified
		if (FireAnimation != NULL)
		{
			// Get the animation object for the arms mesh
			UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
			if (AnimInstance != NULL)
			{
				AnimInstance->Montage_Play(FireAnimation, 1.f);
			}
		}
	}
	else if (CurrentAmmoLoaded <= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No ammo loaded!"));
	}
	else if (ProjectileClass == NULL)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ProjectileClass Class = NULL!"));
	}
}

void ATGPSoloCharacter::ToggleGrenade()
{
	sticky = !sticky;
}

void ATGPSoloCharacter::OnThrowStart()
{
	if (CurrentGrenades > 0)
	{
		grenadeHeld = true;
		justThrown = false;
	}
}


void ATGPSoloCharacter::OnThrowEnd()
{
	if (GrenadeClass != NULL && sinceLastThrow > 0.5 && !justThrown && CurrentGrenades > 0)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			const FRotator SpawnRotation = GetControlRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

			// spawn the projectile at the muzzle
			AGrenade* currentGrenade = World->SpawnActor<AGrenade>(GrenadeClass, SpawnLocation, SpawnRotation, ActorSpawnParams);
			if (currentGrenade != nullptr)
			{
				currentGrenade->SetDuration(heldTime);
				currentGrenade->ProjectileMovement->bShouldBounce = !sticky;
			}
		}
		sinceLastThrow = 0.0f;
		CurrentGrenades--;
	}

	grenadeHeld = false;
	heldTime = 0.0f;
}

void ATGPSoloCharacter::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ATGPSoloCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == true)
	{
		return;
	}
	if ((FingerIndex == TouchItem.FingerIndex) && (TouchItem.bMoved == false))
	{
		OnFire();
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void ATGPSoloCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	TouchItem.bIsPressed = false;
}

void ATGPSoloCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void ATGPSoloCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
	
}

void ATGPSoloCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ATGPSoloCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

float ATGPSoloCharacter::GetCurrentHealth()
{
	return CurrentHealth;
}

float ATGPSoloCharacter::UpdateHealth(float HealthChangeAmount)
{
	CurrentHealth = CurrentHealth + HealthChangeAmount;
	FString currHealth = FString::SanitizeFloat(CurrentHealth);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, currHealth);
	if (CurrentHealth < 0 && !killed)
	{
		GEngine->AddOnScreenDebugMessage(-1, 9999999.9f, FColor::Red, TEXT("You are dead."));
		killed = true;
	}
	return CurrentHealth;
}

float ATGPSoloCharacter::GetCurrentStamina()
{
	return CurrentStamina;
}

float ATGPSoloCharacter::UpdateStamina(float StaminaChangeAmount)
{
	CurrentStamina = CurrentStamina + StaminaChangeAmount;
	return CurrentStamina;
}

int ATGPSoloCharacter::GetCurrentAmmo()
{
	return CurrentAmmo;
}

int ATGPSoloCharacter::UpdateAmmo(int AmmoChangeAmount)
{
	CurrentAmmo = CurrentAmmo + AmmoChangeAmount;
	return CurrentAmmo;
}

void ATGPSoloCharacter::Reload()
{
	if (CurrentAmmoLoaded != magCapacity)
	{
		CurrentAmmo += CurrentAmmoLoaded;
		timeSinceLastShot = -1.0f;

		if (CurrentAmmo > magCapacity)
		{
			CurrentAmmoLoaded = magCapacity;
			CurrentAmmo -= magCapacity;
		}
		else
		{
			CurrentAmmoLoaded = CurrentAmmo;
			CurrentAmmo = 0;
		}
		//return CurrentAmmo;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Reloaded!"));
	}
}

void ATGPSoloCharacter::ToggleFireRate()
{
	FullAuto = !FullAuto;
}

void ATGPSoloCharacter::SprintStart()
{
	GetCharacterMovement()->MaxWalkSpeed *= sprintMultiplier;

}

void ATGPSoloCharacter::SprintEnd()
{
	GetCharacterMovement()->MaxWalkSpeed /= sprintMultiplier;
}

void ATGPSoloCharacter::UpdateEnemiesKilled()
{
	enemiesKilled ++;
	//FString enemiesKilledOutput = FString::FromInt(enemiesKilled);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enemies Killed: ") + enemiesKilledOutput);
}

void ATGPSoloCharacter::UpdateHostagesKilled()
{
	hostagesKilled++;
	//FString hostagesKilledOutput = FString::FromInt(hostagesKilled);
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Hostages Killed: ") + hostagesKilledOutput);
}
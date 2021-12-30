// Fill out your copyright notice in the Description page of Project Settings.


#include "Shooter.h"
#include "..\Public\Shooter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapon.h"
#include "..\Public\Component\HealthComponent.h"
#include "C:\C++ Dev\coop_game\Source\coop_game\coop_game.h"
#include "Net/UnrealNetwork.h"



// Sets default values
AShooter::AShooter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->bUsePawnControlRotation = true;
	SpringArmComp->SetupAttachment(RootComponent);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;
	//GetMovementComponent()->GetNavAgentPropertiesRef().bCanJump = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(COLLISION_WEAPON, ECR_Ignore);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	HPComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));

	ZoomFOV = 65.0f;
	ZoomInterpSpeed = 20;

	WeaponAttachSocketName = "WeaponSocket";
}

// Called when the game starts or when spawned
void AShooter::BeginPlay()
{
	Super::BeginPlay();
	
	DefaultFOV = CameraComp->FieldOfView;
HPComp->OnHealthChanged.AddDynamic(this, &AShooter::OnHealthChanged);

        if (GetLocalRole() == ROLE_Authority) {

    FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CurrentWeapon = GetWorld()->SpawnActor<AWeapon>(StarterWeaponClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (CurrentWeapon) {

		CurrentWeapon->SetOwner(this);
		CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, WeaponAttachSocketName);
	}
        }

	
	
}

void AShooter::MoveForward(float value)
{
	AddMovementInput(GetActorForwardVector() * value);
}

void AShooter::MoveRight(float value)
{
	AddMovementInput(GetActorRightVector() * value);
}

void AShooter::BeginCrouch()
{
	Crouch();
}

void AShooter::EndCrouch()
{
	UnCrouch();
}

void AShooter::BeginZoom()
{
	bWantsToZoom = true;
}

void AShooter::EndZoom()
{
	bWantsToZoom = false;
}

void AShooter::StartFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StartFire();
	}
}

void AShooter::StopFire()
{
	if (CurrentWeapon) {
		CurrentWeapon->StopFire();
	}
}

void AShooter::OnHealthChanged(UHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (Health <= 0.0f && !bDied) {

		bDied = true;

		GetMovementComponent()->StopMovementImmediately();

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		DetachFromControllerPendingDestroy();

		SetLifeSpan(4.0f);

		

	}
}

// Called every frame
void AShooter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFOV = bWantsToZoom ? ZoomFOV : DefaultFOV;

	float NewFOV = FMath::FInterpTo(CameraComp->FieldOfView, TargetFOV, DeltaTime, ZoomInterpSpeed);

	CameraComp->SetFieldOfView(NewFOV);

}

// Called to bind functionality to input
void AShooter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AShooter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AShooter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &AShooter::AddControllerYawInput);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &AShooter::EndCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);

	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &AShooter::BeginZoom);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &AShooter::EndZoom);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooter::StartFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooter::StopFire);
}

FVector AShooter::GetPawnViewLocation() const
{
	if (CameraComp) {
		return CameraComp->GetComponentLocation();
	}

	return Super::GetPawnViewLocation();


}

void AShooter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooter, CurrentWeapon);
	DOREPLIFETIME(AShooter, bDied);
}


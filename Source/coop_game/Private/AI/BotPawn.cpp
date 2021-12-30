// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BotPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include <GameFramework/Character.h>
#include <DrawDebugHelpers.h>
#include "..\Public\Component\HealthComponent.h"
#include "Particles/ParticleSystem.h"
#include "Components/SphereComponent.h"
#include "Shooter.h"




// Sets default values
ABotPawn::ABotPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCanEverAffectNavigation(false);
	RootComponent = MeshComp;

	HealthBot = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	HealthBot->OnHealthChanged.AddDynamic(this, &ABotPawn::HandleTakeDamage);

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComp->SetSphereRadius(200);
	SphereComp->SetupAttachment(RootComponent);
	SphereComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComp->SetupAttachment(RootComponent);

	bUseVelocityChange = false;
	MovementForce = 1000;
	RequiredDistanceToTarget = 100;
	baseDamage = 100;
	DamageRadius = 200;
}

// Called when the game starts or when spawned
void ABotPawn::BeginPlay()
{
	Super::BeginPlay();

	NextPathPoint = GetNextPathPoint();

}

void ABotPawn::HandleTakeDamage(UHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (MatInst == nullptr) {
		MatInst = MeshComp->CreateAndSetMaterialInstanceDynamicFromMaterial(0, MeshComp->GetMaterial(0));

	}

	if (MatInst) {
		MatInst->SetScalarParameterValue("LastDamage", GetWorld()->TimeSeconds);
	}

	if (Health <= 0.0f) {
		SelfDestruct();
	}

	UE_LOG(LogTemp, Log, TEXT("Health %s of %s"), *FString::SanitizeFloat(Health), *GetName());
}

FVector ABotPawn::GetNextPathPoint()
{
	ACharacter* PlayerPawn = UGameplayStatics::GetPlayerCharacter(this, 0);

	UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), PlayerPawn);

	if (NavPath->PathPoints.Num() > 1)
	{
		return NavPath->PathPoints[1];
	}
	return GetActorLocation();

}

void ABotPawn::SelfDestruct()
{
	if (bExploded) {
		return;
	}
	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BotExplode, GetActorLocation());

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);

	UGameplayStatics::ApplyRadialDamage(this, baseDamage, GetActorLocation(), DamageRadius, nullptr, IgnoredActors, this, GetInstigatorController(), true);

	Destroy();


}

void ABotPawn::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ABotPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float DistanceTOTarget = (GetActorLocation() - NextPathPoint).Size();

	if (DistanceTOTarget <= RequiredDistanceToTarget)
	{
		NextPathPoint = GetNextPathPoint();
		DrawDebugString(GetWorld(), GetActorLocation(), "Target Reached!");
	}
	else
	{
		FVector ForceDirection = NextPathPoint - GetActorLocation();
		ForceDirection.Normalize();

		ForceDirection *= MovementForce;

		MeshComp->AddForce(ForceDirection, NAME_None, bUseVelocityChange);

		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);
	}
	DrawDebugSphere(GetWorld(), NextPathPoint, 20, 12, FColor::Yellow, false, 0.0f, 1.0f);
}

void ABotPawn::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if (!bStartedSelfDamage) {
		AShooter* PlayerPawn = Cast<AShooter>(OtherActor);
		if (PlayerPawn) {
			GetWorldTimerManager().SetTimer(TimeHandle_SelfDamage, this, &ABotPawn::DamageSelf, 0.5f, true, 0.0f);
		}
	}

}

// Called to bind functionality to input

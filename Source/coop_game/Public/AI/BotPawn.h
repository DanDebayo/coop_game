// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BotPawn.generated.h"

class UStaticMeshComponent;
class UHealthComponent;
class UParticleSystem;
class USphereComponent;

UCLASS()
class COOP_GAME_API ABotPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABotPawn();

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		UStaticMeshComponent* MeshComp;


	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
		UHealthComponent* HealthBot;

	UPROPERTY(VisibleDefaultsOnly, Category = "Component")
		USphereComponent* SphereComp;

	UFUNCTION()
	void HandleTakeDamage(UHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);

	FVector GetNextPathPoint();
	FVector NextPathPoint;

	UPROPERTY(EditDefaultsOnly, Category = "BotPawn")
	float MovementForce;
	UPROPERTY(EditDefaultsOnly, Category = "BotPawn")
	bool bUseVelocityChange;
	UPROPERTY(EditDefaultsOnly, Category = "BotPawn")
	float RequiredDistanceToTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components")
	UParticleSystem* BotExplode;

	UMaterialInstanceDynamic* MatInst;

	void SelfDestruct();

	bool bExploded;

	bool bStartedSelfDamage;

	UPROPERTY(EditDefaultsOnly, Category = "BotPawn")
	float baseDamage;
	UPROPERTY(EditDefaultsOnly, Category = "BotPawn")
	float DamageRadius;

	FTimerHandle TimeHandle_SelfDamage;

	void DamageSelf();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

};

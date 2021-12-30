// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "GrenadeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class COOP_GAME_API AGrenadeWeapon : public AWeapon
{
	GENERATED_BODY()

protected:

	
	

public:

virtual void Fire() override;

	UPROPERTY(EditDefaultsOnly, Category = "Grenade Weapon")
		TSubclassOf<AActor> ProjectileClass;
};

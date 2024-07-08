#pragma once

#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "TestProjectileSetting.h"
#include "TestProjectile.generated.h"

class UBoxComponent;

UCLASS()
class ATestProjectile : public AActor
{
    GENERATED_BODY()

public:
    ATestProjectile();

protected:
    virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY()
    UArrowComponent* ArrowComponent;

    UPROPERTY()
    UBoxComponent* CollisionComponent;

protected:
    float Speed = 0.0f;
    FVector PositionStart;
    FVector Direction;
    FQuat Rotation;

    float TimeSpawned = 0.0f;
    float TimeDestroyForce = 0.0f;
    bool IsHitReflect = false;

public:
	TFunction<void(ATestProjectile*, float)> OnTimeTrigger = nullptr;

public:
	static TSet<ATestProjectile*> Projectiles;

public:
	static void DestroyRequestAll();

protected:
    bool IntersectLine(FHitResult& OutHit, const FVector& InStart, const FVector& InEnd);

public:
    virtual void InitProjectile(const FVector& InPosition, FVector& InDirection, float InScale, bool bIsHitReflect = false, float InTimeDestroy = DEFAULT_DESTROY, FColor InColor = FColor::Red);

protected:
    virtual void UpdateLocation(const FVector& LocationCurrent, float DeltaTime);
    virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
};

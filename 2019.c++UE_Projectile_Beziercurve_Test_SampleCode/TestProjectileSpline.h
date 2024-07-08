#pragma once
#include "TestProjectile.h"
#include "Components/SplineComponent.h"
#include "TestProjectileSpline.generated.h"

UCLASS()
class ATestProjectileSpline : public ATestProjectile
{
	GENERATED_BODY()

public:
	ATestProjectileSpline();

public:
	UPROPERTY()
	USplineComponent* SplineComponent;

	float TimeCumulative = 0.0f;

public:
	virtual void InitProjectile(const FVector& InPosition, FVector& InDirection, float InScale, bool bIsHitReflect = false, float InTimeDestroy = DEFAULT_DESTROY, FColor InColor = FColor::Red) override;

protected:
	virtual void UpdateLocation(const FVector& LocationCurrent, float DeltaTime) override;
};


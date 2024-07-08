#include "TestProjectileSpline.h"
#include "TestUE4_01Character.h"

ATestProjectileSpline::ATestProjectileSpline()
{
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
}

void ATestProjectileSpline::InitProjectile(const FVector& InPosition, FVector& InDirection, float InScale, bool bIsHitReflect, float InTimeDestroy, FColor InColor)
{
	GWarn->Logf(ELogVerbosity::Display, TEXT(" AddSplineWorldPoint(%s)"), *InPosition.ToString());

	SplineComponent->ClearSplinePoints();
	SplineComponent->AddSplineLocalPoint(InPosition);
	const int countIteration = 10;
	const float distance = DEFAULT_DIST_PROJECTILESPLINE, distanceUnit = distance / (float)countIteration, distanceWave = 50.0f;
	for (int i = 0; i < countIteration; ++i)
	{
		float currentDistance = distanceUnit * i;
		FVector currentPosition = InPosition + (InDirection * currentDistance);
		currentPosition.Z = currentPosition.Z + ((0 == i % 2) ? distanceWave : -distanceWave);
		SplineComponent->AddSplineLocalPoint(currentPosition);

		GWarn->Logf(ELogVerbosity::Display, TEXT(" AddSplineWorldPoint(%s)"), *currentPosition.ToString());
	}

	TimeCumulative = 0.0f;

	Super::InitProjectile(InPosition, InDirection, InScale, bIsHitReflect, InTimeDestroy, InColor);	
}

void ATestProjectileSpline::UpdateLocation(const FVector& LocationCurrent, float DeltaTime)
{
	float time = TimeCumulative / (DEFAULT_DIST_PROJECTILESPLINE/DEFAULT_SPEED_PROJECTILE);
	FVector newLocation = SplineComponent->GetLocationAtTime(time, ESplineCoordinateSpace::World);
	SetActorLocationAndRotation(newLocation, Rotation, false, 0, ETeleportType::None);

	TimeCumulative = TimeCumulative + DeltaTime;

	if (time > 1.0f)
	{
		ATestUE4_01Character::AddDestroyRequest(this);
	}
}

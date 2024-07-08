#include "TestProjectile.h"
#include "CoreMinimal.h"
#include "TestUE4_01Character.h"
#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"

TSet<ATestProjectile*> ATestProjectile::Projectiles;

ATestProjectile::ATestProjectile()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    CollisionComponent->SetVisibleFlag(true);
    CollisionComponent->bHiddenInGame = false;
    CollisionComponent->InitBoxExtent(FVector(45, 7, 7));
    CollisionComponent->AddRelativeLocation(FVector(DEFAULT_ARROW_LEN, 0, 0));
	CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
    ArrowComponent->bHiddenInGame = false;
    ArrowComponent->bUseAttachParentBound = false;
    ArrowComponent->AddRelativeLocation(FVector(-45, 0, 0));
    ArrowComponent->SetupAttachment(CollisionComponent);

    RootComponent = CollisionComponent;
}

void ATestProjectile::BeginPlay()
{
    Super::BeginPlay();
	Projectiles.Add(this);
}

void ATestProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	Projectiles.Remove(this);
}

void ATestProjectile::Tick(float DeltaTime)
{
    if (FPlatformTime::Seconds() >= TimeDestroyForce)
    {
        ATestUE4_01Character::AddDestroyRequest(this);
        ArrowComponent->bHiddenInGame = true;
        return;
    }

    Super::Tick(DeltaTime);
    UpdateLocation(GetTransform().GetLocation(), DeltaTime);
	if (nullptr != OnTimeTrigger)
	{
		float elapsed = FPlatformTime::Seconds() - TimeSpawned;
		OnTimeTrigger(this, elapsed);
	}
}

void ATestProjectile::DestroyRequestAll()
{
	for (ATestProjectile* Iter : Projectiles)
	{
		ATestUE4_01Character::AddDestroyRequest(Iter);
	}
}

bool ATestProjectile::IntersectLine(FHitResult& OutHit, const FVector& InStart, const FVector& InEnd)
{
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActors(ATestUE4_01Character::FirstActors);
	TraceParams.bTraceComplex = true;

	bool intersected = GWorld->LineTraceSingleByObjectType(OutHit, InStart, InEnd, FCollisionObjectQueryParams(ECC_WorldStatic), TraceParams);
	return intersected;
}

void ATestProjectile::InitProjectile(const FVector& InPosition, FVector& InDirection, float InScale, bool bIsHitReflect, float InTimeDestroy, FColor InColor)
{
    Direction = InDirection;
    Rotation = Direction.ToOrientationQuat();
    Speed = DEFAULT_SPEED_PROJECTILE;
    TimeSpawned = FPlatformTime::Seconds();
    TimeDestroyForce = TimeSpawned + InTimeDestroy;
    PositionStart = InPosition + (InDirection * DEFAULT_RELATIVE);
    
    IsHitReflect = bIsHitReflect;
    CollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    if (true == bIsHitReflect)
    {
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		TraceParams.AddIgnoredActors(ATestUE4_01Character::FirstActors);
		TraceParams.bTraceComplex = true;
        
		FVector boxExtent = Rotation.RotateVector(CollisionComponent->GetScaledBoxExtent()).GetAbs();
		FCollisionShape collisionShape; 
		collisionShape.SetBox(FVector3f(boxExtent));

		TArray<FOverlapResult> OutOverlaps;

		bool overlapped = GWorld->OverlapMultiByObjectType(OutOverlaps, PositionStart + (Direction * boxExtent.Y * 0.5f), FQuat::Identity, FCollisionObjectQueryParams(ECC_WorldStatic), collisionShape, TraceParams);
		if (true == overlapped)
		{
			for (int i = 0; i < OutOverlaps.Num(); ++i)
			{
				FOverlapResult& overlap = OutOverlaps[i];
				if (nullptr != overlap.GetActor() && overlap.GetActor()->GetClass() != ATestProjectile::StaticClass())
				{
					if (true == IsHitReflect)
					{
						Rotation = Rotation * -1.0f;
						break;
					}
					else
					{
						ATestUE4_01Character::AddDestroyRequest(this);
						continue;
					}
				}
			}	
		}
    }

	ArrowComponent->SetArrowColor(InColor);

    SetActorScale3D(FVector(InScale));
    SetActorLocationAndRotation(PositionStart, Rotation, false, 0, ETeleportType::None);
}

void ATestProjectile::UpdateLocation(const FVector& LocationCurrent, float DeltaTime)
{
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActors(ATestUE4_01Character::FirstActors);
	TraceParams.bTraceComplex = true;
	FHitResult Hit;
	bool Hitted = GWorld->LineTraceSingleByObjectType(Hit, LocationCurrent, LocationCurrent + (Direction * DEFAULT_ARROW_LEN), FCollisionObjectQueryParams(ECC_WorldStatic), TraceParams);
	if (true == Hitted && Hit.GetActor()->GetClass() != ATestProjectile::StaticClass())
	{
		if (true == IsHitReflect)
		{
			//GWarn->Logf(ELogVerbosity::Error, TEXT(" IsHitCheckForce/Reflect:%s/"), *Hit.GetActor()->GetName());
			Direction = Direction - ((FVector::DotProduct(Direction, Hit.Normal) * 2) * Hit.Normal);
			//Direction = FMath::GetReflectionVector(Direction, Hit.Normal);
		}
		else
		{
			ATestUE4_01Character::AddDestroyRequest(this);
			ArrowComponent->bHiddenInGame = true;
			return;
		}
	}

	FVector NewLocation = LocationCurrent + (Direction * (Speed * DeltaTime));
	Rotation = Direction.ToOrientationQuat();
	SetActorLocationAndRotation(NewLocation, Rotation, false, 0, ETeleportType::None);
}

void ATestProjectile::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    GWarn->Logf(ELogVerbosity::Error, TEXT(" ATestProjectile::NotifyHit/%s/"), *Other->GetName());
}

#pragma once

#include "Carla/Sensor/Sensor.h"
#include "Carla/Sensor/Radar.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/geom/Vector3D.h"
#include <compiler/enable-ue4-macros.h>

#include "Sensor/RayCastSemanticLidar.h"
#include "Keti/SensorDescription/KetiLidarDescription.h"
#include "KetiSensorCaptureRay.h"
#include "KetiSensorCommon.h"
#include "KetiSensorCapture.h"
#include "KetiSensorEventBase.h"
#include "KetiSensorRayCastLidar.generated.h"

class USceneCaptureComponent2D;
class ACarlaHUD;
class ACustomRTRayRunner;

using Vector3D = carla::geom::Vector3D;

UCLASS()
class CARLA_API AKetiSensorRayCastLidar : public ARayCastSemanticLidar, public UKetiSensorEventBase
{
    GENERATED_BODY()
    
public:
    static FActorDefinition GetSensorDefinition();
    inline const static FString TypeNameDescription = TEXT("sensor.lidar.keti_ray_cast_lidar");  

    AKetiSensorRayCastLidar(const FObjectInitializer &ObjectInitializer);

    static void UpdateLoadToActiveDescription(const FLidarDescription& loadDescription, FKetiLidarDescription& LidarOut);

    static FRotator MakeRotator(float pitch, float yaw, float roll)
    {
        return FRotator(pitch, yaw, roll);
    }
    
    void RecreateLaserAngles();
    void RecreateChannelAngles();

    enum ELidarUpdateType { None, RecareateLasers, UpdateOnlyLasers };
    void UpdateDescription(ELidarUpdateType updateType = ELidarUpdateType::None);

    virtual uint32 GetActorID() override;

    UFUNCTION()
    void EventEndPlayVehicle(AActor* vehicle, EEndPlayReason::Type endPlayReason);
    virtual void RegisterCustomEvents(AActor* vehicle) override;
    virtual void UnRegisterCustomEvents(AActor* vehicle) override;
    
    virtual void Set(const FActorDescription &Description) override;
    virtual void BeginPlay() override;
protected:
    
    virtual void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime) override;
    void TickCapture(UWorld *World, const float DeltaTime);
    void TickCaptureTest(UWorld *World, const float DeltaTime);
    bool GetColorFromCapture(  const FKetiSensorCapture& capture,
                                const FRotator& rotatorRay,
                                const FVector& locHit,
                                const TArray<FColor>& bitmap,
                                FColor& color_out,
                                FIntPoint& uv_out);
    bool GetColorCaptured(  const FKetiSensorCapture& capture,
                            const FRotator& rotatorRay,
                            const FVector& locHit,
                            const TArray<FColor>& bitmap,
                            FColor& color_out,
                            FIntPoint& uv_out);
    
    bool ArrangeRayToAroundCapture(FCaptureRay& cast, TArray<FKetiSensorCapture>& aroundCaptures, int32& indexpack_out);
    FCaptureRay* ArrangeRayToAroundCaptureNew(FHitResult& hit, FTransform& tmSensor,
                            TArray<FKetiSensorCapture>& aroundCaptures,
                            int32& indexpack_out);

    
    void TickCaptureTextureRayTest(UWorld *World, const float DeltaTime);
    void TickCaptureMakeTextureRayTest(UWorld *World, const float DeltaTime);
    void TickCaptureAroundRotationTest(UWorld *World, const float DeltaTime);
    void TickCaptureAroundRotation(UWorld *World, const float DeltaTime);
    void TickVisualizeByLineBatcherRTRay(UWorld *World, const float DeltaTime);
    void TickVisualizeByLineBatcher(UWorld *World, const float DeltaTime);
    void TickVisualizeByInstancedMesh(UWorld *World, const float DeltaTime);
    void TickVisualize(UWorld *World, const float DeltaTime);

    
    void SimulateLidarAround(const UWorld *World, const float DeltaTime);
    void SimulateLidarAroundRTRayBuffer(const UWorld *World, const float DeltaTime);
    
    bool ShootLaserKeti(const FTransform& tmSensor, const float VerticalAngle, float HorizontalAngle, FHitResult &HitResult, FCollisionQueryParams& traceParams) const;
    bool ShootLaserKeti(const float VerticalAngle, float HorizontalAngle, FHitResult &HitResult, FCollisionQueryParams& traceParams) const;
    bool ShootLaserKetiRTRayBuffer(const uint32 idxV, const uint32 idxH, const float vAngle, const float hAngle, const FVector& locationSensor, FHitResult &HitResult) const;
    
    void SimulateLidarRTRayProjectionTest(const UWorld *World, const float DeltaTime);

    void MakeOneLaserWorldDirection(const FRotator& sensorRot, const float angleV, float angleH, FVector3f& laser_out);
    void MakeSimulateRTRayBuffer();
    
    FHitResult& WritePointAsyncKeti(uint32_t channel, FHitResult &detection);
    
    /// Compute the received intensity of the point
    void ComputeDetectionSingleKeti(const FHitResult& HitInfo, const FTransform& SensorTransf, FDetection& detection_out) const;
    FDetection ComputeDetectionSingleCarla(const FHitResult& HitInfo, const FTransform& SensorTransf) const;

    void PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel) override;
    bool PostprocessDetection(FDetection& Detection) const;
    bool PostprocessDetectionForKeti(FDetection& Detection, const FHitResult& hit, float range, FVector& noise_out);

    void ResetLidarRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel);
    
    void ComputeAndSaveDetectionsKeti(const FTransform &SensorTransform);
    void ComputeAndSaveDetectionsCarla(const FTransform &SensorTransform);
    
    void ComputeAndSaveDetections(const FTransform &SensorTransform) override;
    void TickComputeIntensity();
    
    void Clear();
    
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void BeginDestroy() override;
#if WITH_EDITORONLY_DATA
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif

private:
    UInstancedStaticMeshComponent* InstancedStaticMeshComp;

public:
    UPROPERTY(EditAnywhere)
    FKetiLidarDescription ActiveDescription;
    
private:
    FKetiSensorCapture TestCaptureSingle;
    TArray<FKetiSensorCapture> AroundCaptures;
    TArray<FCaptureRay> TestCaptureRays;

    TArray<FVector> TestRTRayPickings;
    
    FVector DirCapturedLastProj;
    float AngleYawProj = 0;
    float AnglePitchProj = 0;

    TArray<FColor> Bitmap;
    ACarlaHUD* CarlaHUD = nullptr;
    int32 CountingSceneCapture;

    // static const int ResolutionHorizontal = 1024; // 20hz
    int ResolutionHorizontal        = 2048;  // 10hz
    float AngleUnitHorizontalLidar  = 360.0f / ResolutionHorizontal;
    float AngleHorizontalStart      = 0.0f;
    float AngleHorizontalCurrent    = 0.0f;
    double AngleHAbsCache           = 0.0;
    FCollisionQueryParams TraceParams = FCollisionQueryParams(FName(TEXT("Laser_Trace")), true, this);
    
    FLidarData KetiLidarData;
    
    /// Enable/Disable general dropoff of lidar points
    bool DropOffGenActive;

    float DropOffAlpha;
    float DropOffBeta;

    int CountingWriteTest = 0;

public:
    //@ 360 Rotation Cache
    float AngleDraw360CacheStart = 0.0f;
    float AngleDraw360Cache = AngleHorizontalCurrent;
    int Index360Capacity = ActiveDescription.Channels * ActiveDescription.ResolutionHorizontal; 
    int Index360Iterator = 0;
    std::vector<std::vector<FHitResult>> LidarRecordedHits;
    
    FVector2D CachePointUIVehicle;
    //TArray<FCaptureRay> CacheDrawCircleRays;    // 360 Cache
    //TArray<FVector2D> CacheDrawUIRayPoints;
    //TArray<FColor> CacheDrawUIRayColors;
    TArray<FCaptureRay> DrawCircleRays;
    TArray<FVector2D> DrawUIRayPoints;
    TArray<FColor> DrawUIRayColors;
    //@ 360 Rotation Cache //

    //@ Lidar Distortion
    FTransform TMSensorAroundViewLast;
    TArray<FKetiSensorCommon::FTMTrack> TMSensorTracks;
    double TimeNowOnTick = 0.0;
    //@ Lidar Distortion //

    //@ Lidar Sonar Rotation
    TArray<FCaptureRay> DrawCircleRingRays;
    TArray<FVector2D> DrawUIRayRingPoints;
    TArray<FColor> DrawUIRayRingColors;
    unsigned int CountTotalLaser = 0;
    unsigned int IndexRing = 0;
    unsigned int CountAroundRing = 0;

    unsigned int IndexLaserRTRayBuffer = 0;
    unsigned int CountLaserRTRayBuffer = 0;
    //@ Lidar Sonar Rotation //

    //@ Lidar RT Ray
    ACustomRTRayRunner* RTRayRunner = nullptr;
    //@ Lidar RT Ray //
};

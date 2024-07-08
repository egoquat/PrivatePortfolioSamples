#include "KetiSensorRayCastLidar.h"
#include "Components/LineBatchComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Game/CarlaHUD.h"
#include "Sensor/PixelReader.h"
#include "ImageWriteQueue.h"
#include "ImageWriteTask.h"
#include "KetiSensorCapturePool.h"
#include "KetiSensorCommon.h"
#include "KetiSensorUtility.h"
#ifdef ACTIVATE_LIDARPOINTCLOUD
#include "LidarPointCloud.h"
#include "IO/LidarPointCloudFileIO.h"
#endif
#include "Async/ParallelFor.h"

#include <compiler/disable-ue4-macros.h>
#include "carla/image/CityScapesPalette.h"
#include "carla/trafficmanager/Constants.h"
#include <compiler/enable-ue4-macros.h>

#include "KetiSensorCollection.h"
#include "Keti/PlayerController/KetiPlayerController.h"
#include "Kismet/KismetMathLibrary.h"
#include "Rendering/SkeletalMeshLODImporterData.h"
#include "Scene/TextRenderBillboardComponent.h"
#include "CustomShader/CustomRTRayRunner.h"

#include <PhysicsEngine/PhysicsObjectExternalInterface.h>

#define TestCaptureON 0
#define CaptureLinearColor 0
#define SecondDrawDefault 0.2f

static int GLidarRotationing = 1;
FAutoConsoleVariableRef CLidarRotationing(
TEXT("Keti.Sensor.Lidar.LidarRotationing"),
GLidarRotationing,
TEXT("default ON = 1\n")
TEXT("Off = 0\n"),
ECVF_Cheat
);

static int GLidarCaptureCntPerSec = 100;
FAutoConsoleVariableRef CVarLidarCaptureCntPerSec(
TEXT("Keti.Sensor.Lidar.CaptureCntPerSec"),
GLidarCaptureCntPerSec,
TEXT("CaptureCntPerSec.\n")
TEXT("default = 1000\n")
TEXT("maximum = 1000000\n"),
ECVF_Cheat
);

static int GLidarShowVisualize = 1;
FAutoConsoleVariableRef CVarLidarShowVisualize(
TEXT("Keti.Sensor.Lidar.ShowVisualize"),
GLidarShowVisualize,
TEXT("default Off = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarShowVisualizeGameWorld = 0;
FAutoConsoleVariableRef CVarShowVisualizeGameWorld(
TEXT("Keti.Sensor.Lidar.ShowVisualizeGameWorld"),
GLidarShowVisualizeGameWorld,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarLidarShowCaptureTexture = 0;
FAutoConsoleVariableRef CVarVisualizeCaptureTexture(
TEXT("Keti.Sensor.Lidar.LidarShowCaptureTexture"),
GLidarLidarShowCaptureTexture,
TEXT("default Off = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarSaveCaptureTextureRequest = 0;
FAutoConsoleVariableRef CVarSaveCaptureTextureRequest(
TEXT("Keti.Sensor.Lidar.SaveCaptureTextureRequest"),
GLidarSaveCaptureTextureRequest,
TEXT("default Off = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static float GLidarTestFisheyeSimpleDistort = 0.8f;
FAutoConsoleVariableRef CTestFisheyeSimpleDistort(
TEXT("Keti.Sensor.Lidar.TestFisheyeSimpleDistort"),
GLidarTestFisheyeSimpleDistort,
TEXT("default float"),
ECVF_Cheat
);

static int GLidarLidarTest = 0;
FAutoConsoleVariableRef CSensorLidarTest(
TEXT("Keti.Sensor.Lidar.LidarTest"),
GLidarLidarTest,
TEXT("default Off = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarLidarTestTypeAround = 1;
FAutoConsoleVariableRef CSensorLidarTestTypeAround(
TEXT("Keti.Sensor.Lidar.LidarTestTypeAround"),
GLidarLidarTestTypeAround,
TEXT("default ON = 1\n")
TEXT("OFF = 0\n"),
ECVF_Cheat
);

static int GLidarTestRaycastDrawDirectly = 0;
FAutoConsoleVariableRef CLidarTestRaycastDrawDirectly(
TEXT("Keti.Sensor.Lidar.LidarTestRaycastDrawDirectly"),
GLidarTestRaycastDrawDirectly,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarCaptureRequest = 0;
FAutoConsoleVariableRef CVarCaptureRequest(
TEXT("Keti.Sensor.Lidar.CaptureRequest"),
GLidarCaptureRequest,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarEmptyCastsRequest = 0;
FAutoConsoleVariableRef CVarEmptyCastsRequest(
TEXT("Keti.Sensor.Lidar.EmptyCastsRequest"),
GLidarEmptyCastsRequest,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarShowDrawLines = 0;
FAutoConsoleVariableRef CVarGLidarShowDrawLines(
TEXT("Keti.Sensor.Lidar.ShowDrawLines"),
GLidarShowDrawLines,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarVisualize360 = 1;
FAutoConsoleVariableRef CVarVisualize360(
TEXT("Keti.Sensor.Lidar.Visualize360"),
GLidarVisualize360,
TEXT("default ON = 1\n")
TEXT("OFF = 0\n"),
ECVF_Cheat
);

static int GLidarVisualize360Sonar = 0;
FAutoConsoleVariableRef CVarVisualize360Sonar(
TEXT("Keti.Sensor.Lidar.Visualize360Sonar"),
GLidarVisualize360Sonar,
TEXT("default ON = 1\n")
TEXT("OFF = 0\n"),
ECVF_Cheat
);

static int GLidarVisualizeSemantic = 0;
FAutoConsoleVariableRef CVarVisualizeSemantic(
TEXT("Keti.Sensor.Lidar.VisualizeSemantic"),
GLidarVisualizeSemantic,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarIntensityUseKeti = 1;
FAutoConsoleVariableRef CVarLidarComputeDetectUseKeti(
TEXT("Keti.Sensor.Lidar.IntensityUseKeti"),
GLidarIntensityUseKeti,
TEXT("default ON = 1\n")
TEXT("OFF = 0\n"),
ECVF_Cheat
);

static float GLidarWeightNoise = 1.0f;
FAutoConsoleVariableRef CVarLidarDistanceRatioNoise(
TEXT("Keti.Sensor.Lidar.DistanceRatioNoise"),
GLidarWeightNoise,
TEXT("default float = 1.0f\n"),
ECVF_Cheat
);

static int GLidarVisualizeByInstanced = 0;
FAutoConsoleVariableRef CVarLidarVisualizeByInstanced(
TEXT("Keti.Sensor.Lidar.VisualizeByInstanced"),
GLidarVisualizeByInstanced,
TEXT("default = 0\n"),
ECVF_Cheat
);

static int GVisualizeTrackToVehicle = 1;
FAutoConsoleVariableRef CVarGVisualizeTrackToVehicle(
TEXT("Keti.Sensor.Lidar.VisualizeTrackToVehicle"),
GVisualizeTrackToVehicle,
TEXT("default = 1\n"),
ECVF_Cheat
);

static int GLidarDistortionLerp = 0;
FAutoConsoleVariableRef CVarGLidarDistortionLerp(
TEXT("Keti.Sensor.Lidar.DistortionLerp"),
GLidarDistortionLerp,
TEXT("default 1, off 0\n"),
ECVF_Cheat
);

static int GLidarMeasurePerformanceRequest = 0;
FAutoConsoleVariableRef CVarMeasurePerformanceRequest(
TEXT("Keti.Sensor.Lidar.MeasurePerformanceRequest"),
GLidarMeasurePerformanceRequest,
TEXT("default OFF = 0\n")
TEXT("ON = 1\n"),
ECVF_Cheat
);

static int GLidarUseCustomRTRay = 0;
FAutoConsoleVariableRef CVarLidarRTRayTest(
TEXT("Keti.Sensor.Lidar.RTRayTest"),
GLidarUseCustomRTRay,
TEXT("default = 1\n"),
ECVF_Cheat
);

static int GLidarSendDataStream = 1;
FAutoConsoleVariableRef CGLidarSendDataStream(
TEXT("Keti.Sensor.Lidar.SendDataStream"),
GLidarSendDataStream,
TEXT("default Off = 1\n"),
ECVF_Cheat
);

typedef FKetiSensorCommon::FTMTrack FTMTrack;

//@ Const Parameter
static const TMap<int32, float> Reflectivities {
	{0 	, 0.5f},    //None = 0u,          
    {1 	, 0.4f},    //Buildings = 1u,     
    {2 	, 0.7f},    //Fences = 2u,        
    {3 	, 0.5f},   	//Other = 3u,         
    {4 	, 0.5f},   	//Pedestrians = 4u,   
    {5 	, 0.8f},   	//Poles = 5u,         
    {6 	, 1.0f},  	//RoadLines = 6u,     
    {7 	, 0.6f}, 	//Roads = 7u,         
    {8 	, 0.7f}, 	//Sidewalks = 8u,     
    {9 	, 0.7f},  	//Vegetation = 9u,    
    {10 	, 0.7f},  	//Vehicles = 10u,     
    {11 	, 0.4f}, 	//Walls = 11u,        
    {12 	, 1.0f},  	//TrafficSigns = 12u, 
    {13 	, 0.0f},  	//Sky = 13u,          
    {14 	, 0.4f}, 	//Ground = 14u,       
    {15 	, 0.4f}, 	//Bridge = 15u,       
    {16 	, 0.6f}, 	//RailTrack = 16u,    
    {17 	, 0.9f}, 	//GuardRail = 17u,    
    {18 	, 0.8f},  	//TrafficLight = 18u, 
    {19 	, 0.5f}, 	//Static = 19u,       
    {20 	, 0.5f},  	//Dynamic = 20u,      
    {21 	, 0.1f},  	//Water = 21u,        
    {22 	, 0.2f},  	//Terrain = 22u,      
    {255 	, 0.5f} 	//Any = 0xFF          
};
//@ Const Parameter //

//@ Test Parameter Capture Division
static const float DistanceDotSt = 10000;
//@ Test Parameter Capture Division //

using FProfiler = FKetiSensorUtility::FProfiler;

FActorDefinition AKetiSensorRayCastLidar::GetSensorDefinition()
{
    bool succeeded = false;
    FActorDefinition Definition;    
    UActorBlueprintFunctionLibrary::MakeKetiLidarDefinition(TEXT("keti_ray_cast_lidar"), succeeded, Definition);
    return Definition;
}

AKetiSensorRayCastLidar::AKetiSensorRayCastLidar(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    RandomEngine = CreateDefaultSubobject<URandomEngine>(TEXT("RandomEngine"));
    SetSeed(ActiveDescription.RandomSeed);

    UStaticMesh* cubeMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), this,
                            TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
    
    InstancedStaticMeshComp = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("UInstancedStaticMeshComponent"));
    //InstancedStaticMeshComp->SetupAttachment(RootComponent);
    InstancedStaticMeshComp->SetStaticMesh(cubeMesh);
}

void AKetiSensorRayCastLidar::UpdateLoadToActiveDescription(const FLidarDescription& loadDescription, FKetiLidarDescription& LidarOut)
{
}

void AKetiSensorRayCastLidar::RecreateLaserAngles() 
{    
    const int32 NumberOfLasers = ActiveDescription.ChannelAngles.Num();
    float angleDelta = 0;
    float angleSum   = 0;
    LaserAngles.SetNum(NumberOfLasers, true);
    
    float ratio = 0, lastratio = 0;
    float angleRangeVertical = (ActiveDescription.UpperFovLimit - ActiveDescription.LowerFovLimit);
    bool hasCurve = ActiveDescription.ChannelCurve.EditorCurveData.GetNumKeys() >= 2;
    float curveTimeFirst = 0, curveTimeLast = 0;
    if (hasCurve == true)
    {
        curveTimeFirst = ActiveDescription.ChannelCurve.EditorCurveData.GetFirstKey().Time;
        curveTimeLast = ActiveDescription.ChannelCurve.EditorCurveData.GetLastKey().Time;
    }
    
    for (int32 i = 0; i < NumberOfLasers; ++i)
    {
        ratio = ((float)i / (float)(NumberOfLasers - 1));
        if (hasCurve)
        {
            ratio = ActiveDescription.ChannelCurve.EditorCurveData.Eval(curveTimeFirst + (ratio * (curveTimeLast - curveTimeFirst)));
            float relativeratio = ratio - lastratio;
            ActiveDescription.ChannelAngles[i] = angleRangeVertical * relativeratio;
            lastratio = ratio;
        }
        angleDelta = ActiveDescription.ChannelAngles[i];
        angleSum = angleSum + angleDelta;
        const float VerticalAngle = ActiveDescription.LowerFovLimit + angleSum;
        LaserAngles[NumberOfLasers - i - 1] = (VerticalAngle);
    }
}

void AKetiSensorRayCastLidar::RecreateChannelAngles()
{
    ActiveDescription.ChannelAngles.Init(0, ActiveDescription.Channels);
    
    const auto NumberOfLasers = ActiveDescription.Channels;
    check(NumberOfLasers > 0u);
    const float DeltaAngle = NumberOfLasers == 1u ? 0.f : (ActiveDescription.UpperFovLimit - ActiveDescription.LowerFovLimit) /
      static_cast<float>(NumberOfLasers - 1);
    
    for(auto i = 0u; i < NumberOfLasers; ++i)
    {
        ActiveDescription.ChannelAngles[i] = DeltaAngle;
    }
}

void AKetiSensorRayCastLidar::UpdateDescription(ELidarUpdateType updateType)
{
    ActiveDescription.ToLidarDescription(Description);

    ResolutionHorizontal = ActiveDescription.ResolutionHorizontal;
    AngleUnitHorizontalLidar = 360.0f / ResolutionHorizontal;
    AngleHorizontalStart = 0;
    
    PointsPerChannel.resize(ActiveDescription.Channels);

    if (updateType == ELidarUpdateType::RecareateLasers)
    {
        RecreateChannelAngles();
        RecreateLaserAngles();
    }
    else if (updateType == ELidarUpdateType::UpdateOnlyLasers)
    {
        RecreateLaserAngles();
    }

    if (KetiLidarData.GetChannelCount() != ActiveDescription.Channels)
    {
        KetiLidarData = FLidarData(ActiveDescription.Channels);
    }
    
    DropOffBeta = 1.0f - ActiveDescription.DropOffAtZeroIntensity;
    DropOffAlpha = ActiveDescription.DropOffAtZeroIntensity / ActiveDescription.DropOffIntensityLimit;
    DropOffGenActive = ActiveDescription.DropOffGenRate > std::numeric_limits<float>::epsilon();
    CountTotalLaser = ResolutionHorizontal * ActiveDescription.Channels;

    FCaptureRay defaultRay;
    DrawCircleRingRays.Init(defaultRay, CountTotalLaser);
    DrawUIRayRingPoints.Init(FVector2D::ZeroVector, CountTotalLaser);
    DrawUIRayRingColors.Init(FColor::Black, CountTotalLaser);
    IndexRing = CountAroundRing = CountLaserRTRayBuffer = 0;

    Index360Capacity = CountTotalLaser;
    Index360Iterator = 0;
    DrawCircleRays.SetNum(Index360Capacity);
    DrawUIRayPoints.SetNum(Index360Capacity);
    DrawUIRayColors.SetNum(Index360Capacity);
}

uint32 AKetiSensorRayCastLidar::GetActorID()
{
    const FActorRegistry& Registry   = GetEpisode().GetActorRegistry();
    const FCarlaActor* view = Registry.FindCarlaActor(this);
    ensure(view);
    if (view)
    {
        return view->GetActorId();
    }
    else
    {
        UE_LOG(LogCarla, Warning, TEXT("Registry.FindCarlaActor(this) %p"), this);
        return 0;
    }
}

void AKetiSensorRayCastLidar::EventEndPlayVehicle(AActor* vehicle, EEndPlayReason::Type endPlayReason)
{
    if (RTRayRunner == nullptr) return;
    
    RTRayRunner->EndRendering();
}

void AKetiSensorRayCastLidar::RegisterCustomEvents(AActor* vehicle)
{
    ensure(vehicle);
    vehicle->OnEndPlay.AddDynamic(this, &AKetiSensorRayCastLidar::EventEndPlayVehicle);
}

void AKetiSensorRayCastLidar::UnRegisterCustomEvents(AActor* vehicle)
{
    ensure(vehicle);
    vehicle->OnEndPlay.RemoveAll(this);
}

void AKetiSensorRayCastLidar::Set(const FActorDescription &ActorDescription)
{    
    FLidarDescription loadDescription;
    UActorBlueprintFunctionLibrary::SetLidar(ActorDescription, loadDescription);

    Description       = loadDescription;

    UpdateLoadToActiveDescription(loadDescription, ActiveDescription);
    UpdateDescription(ELidarUpdateType::RecareateLasers);
}

void AKetiSensorRayCastLidar::BeginPlay()
{
    Super::BeginPlay();
    
    TMSensorTracks.Add(FTMTrack(FPlatformTime::Seconds(), TMSensorAroundViewLast));
    
    SetActorEnableCollision(false);
    
    UStaticMeshComponent* meshComp = Cast<UStaticMeshComponent>(RootComponent);
    if (meshComp)
    {
        meshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        //meshComp->SetHiddenInGame(true);
    }
    
    float pitchMid = (ActiveDescription.LowerFovLimit + ActiveDescription.UpperFovLimit) / 2;
    AroundCaptures.Empty();
    float yawLookAt = 0;
    float pitchLookAt = pitchMid;
    FRotator rotatorCaptureLookAt(pitchLookAt, yawLookAt, 0);
    for (int i = 0; i < FKetiSensorCommon::CaptureDivFOVAngle; ++i)
    {
        UTextureRenderTarget2D* renderTargetNew;
        rotatorCaptureLookAt.Yaw = ((float)i * FKetiSensorCommon::CaptureFOVAngleUnit);
        USceneCaptureComponent2D* captureCompNew = FKetiSensorCapture::MakeSceneCaptureRT(this,
                                                rotatorCaptureLookAt, FKetiSensorCommon::CaptureFOVAngleUnit,
                                                FKetiSensorCommon::LidarCaptureResolution.X,
                                                FKetiSensorCommon::LidarCaptureResolution.Y,
                                                true, renderTargetNew);
        AroundCaptures.Add(FKetiSensorCapture(i, FKetiSensorCommon::LidarCaptureResolution,
                                        FKetiSensorCommon::CaptureFOVAngleUnit,
                                        rotatorCaptureLookAt.Yaw,
                                        pitchLookAt,
                                        captureCompNew,
                                        renderTargetNew));
        FKetiSensorCapturePool::AddCapture(this, captureCompNew);
    }

#if TestCaptureON == 1
    TestCaptureSingle = FKetiSensorCapture(0, FKetiSensorCommon::LidarCaptureResolution,
                                            FKetiSensorCommon::CaptureFOVAngleUnit, 0, pitchMid, nullptr, nullptr);
    TestCaptureSingle.CaptureComp = FKetiSensorCapture::MakeSceneCaptureRT(this,
                                    rotatorCaptureLookAt, FKetiSensorCommon::CaptureFOVAngleUnit,
                                    FKetiSensorCommon::LidarCaptureResolution.X,
                                    FKetiSensorCommon::LidarCaptureResolution.Y, true, TestCaptureSingle.RenderTarget);
    TestCaptureSingle.OnUpdateCapture();
#endif

    AngleHorizontalCurrent = AngleHorizontalStart = 0.0f;
    AngleHAbsCache = AngleDraw360CacheStart = AngleDraw360Cache = 0.0f;

    TraceParams.bTraceComplex = true;
    TraceParams.bReturnPhysicalMaterial = true;
    TraceParams.bReturnFaceIndex = false;
    TraceParams.AddIgnoredActor(this);
    
    Bitmap.Empty();
    Bitmap.AddZeroed(FKetiSensorCommon::LidarCaptureResolution.X * FKetiSensorCommon::LidarCaptureResolution.X);

    UWorld* world = GetWorld();
    if (FKetiSensorCommon::GActivateCarlaHUD == 1 && CarlaHUD == nullptr)
    {
        APlayerController *playercontroller = UGameplayStatics::GetPlayerController(world, 0);
        if (playercontroller)
        {
            AHUD* hud = playercontroller->GetHUD();
            CarlaHUD = Cast<ACarlaHUD>(hud);
            ensure(CarlaHUD);
        }
        
        CarlaHUD->SensorHud.AddInstanceLidar(this,
                        GetOwner(),
                        FSensorHudInstance::FarCaptureDefault,
                        ActiveDescription.RangeMax,
                        DrawUIRayPoints,
                        DrawUIRayColors,
                        &FKetiSensorCommon::GActivateLidar);
    }
    
    AKetiPlayerController* pc = Cast<AKetiPlayerController>(world->GetFirstPlayerController());
    ensure(pc);
    FSensorCollection::PostNewInstanceSensorKeti(this);

    TMSensorAroundViewLast = TMSensorTracks.Last().TMTrack;
    
    //RTRaycast
    RTRayRunner = world->SpawnActorDeferred<ACustomRTRayRunner>( ACustomRTRayRunner::StaticClass(), GetTransform(), this);
    RTRayRunner->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform); 
    RTRayRunner->SetActorRelativeLocation(FVector::ZeroVector);
    RTRayRunner->Initialize(this, ActiveDescription.ResolutionHorizontal, ActiveDescription.Channels);
    MakeSimulateRTRayBuffer();
}

void AKetiSensorRayCastLidar::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_LidarTick);

    if (FKetiSensorCommon::GActivateLidar == 0) return;
    
    FProfiler::Reset();
    
    FProfiler::Begin(0);
    
    TimeNowOnTick = FPlatformTime::Seconds();    
    
    if (GLidarUseCustomRTRay == 1 && RTRayRunner)
    {
        FProfiler::Begin(9);
        RTRayRunner->ExternalTick(this, DeltaTime);
        FProfiler::End(9);
    }
    
    FKetiSensorCapturePool::ActivateCaptures(this, FKetiSensorCommon::GActivateLidar == 1);
    if (CarlaHUD)
    {
        CarlaHUD->SensorHud.SetVisible(this, FKetiSensorCommon::GActivateLidar != 0);
    }
    
    if (GLidarLidarTest == 1)
    {
        TickCaptureTest(World, DeltaTime);
    }
    else
    {
        if (GLidarRotationing != 0)
        {
            FProfiler::Begin(1);
            if (GLidarUseCustomRTRay == 1)
            {
                //SimulateLidarRTRayProjectionTest(World, DeltaTime);
                //MakeSimulateRTRayBuffer();
                //GLidarUseCustomRTRay = 0;
                SimulateLidarAroundRTRayBuffer(World, DeltaTime);
            }
            else
            {
                SimulateLidarAround(World, DeltaTime);
            }

            if (TestRTRayPickings.Num() >= 1)
            {
                ULineBatchComponent* LineBatcher = World->LineBatcher;
                for (int i = 0; i < TestRTRayPickings.Num(); ++i)
                {
                    FVector& pos = TestRTRayPickings[i];
                    LineBatcher->DrawPoint(pos, FColor::Cyan, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                }
            }
            
            FProfiler::End(1);

            FProfiler::Begin(2);
            TickCapture(World, DeltaTime);
            FProfiler::End(2);
        }
    }

    FProfiler::Begin(3);
    TickComputeIntensity();
    FProfiler::End(3);
    
    FProfiler::Begin(4);
    if (GLidarShowVisualize == 1)
    {
        TickVisualize(World, DeltaTime);
    }
    FProfiler::End(4);
    
    if (GLidarSendDataStream == 1)
    {
        auto DataStream = GetDataStream(*this);
        DataStream.Send(*this, KetiLidarData, DataStream.PopBufferFromPool());
    }
    
    TMSensorTracks.RemoveAll([&](FTMTrack& iter)
        { return FMath::Abs(TimeNowOnTick - iter.TimeTrack) > FKetiSensorCommon::TimeLimitTrack; });
    TMSensorTracks.Add(FTMTrack(TimeNowOnTick, GetTransform()));

    FProfiler::End(0);
    
    if (GLidarMeasurePerformanceRequest == 1)
    {
        GLog->Logf(TEXT("Profiler Lidar SimulateLidar:%f/TickCapture:%f/EstCountRay:%d/EstCountCast:%d/EstWriteRay:%d"),
            FProfiler::GetTimeElapsed(1),
            FProfiler::GetTimeElapsed(2),
            (int)(FProfiler::CountingRay * (1.0f / DeltaTime)),
            (int)(FProfiler::CountingRaycast * (1.0f / DeltaTime)),
            (int)(CountingWriteTest * (1.0f / DeltaTime)));
    
        FProfiler::OutputProfilerDefault();
            
        GLidarMeasurePerformanceRequest = 0;
    }
}

void AKetiSensorRayCastLidar::TickCapture(UWorld *World, const float DeltaTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(STAT_LidarTickCapture);
    TickCaptureAroundRotation(World, DeltaTime);
    
    if (GLidarSaveCaptureTextureRequest == 1)
    {
        for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
        {
            FKetiSensorCapture& capture = AroundCaptures[icapture];
            FString pathSaveTextureTest = FString::Printf(TEXT("Captured_Test_360_%d.bmp"), icapture);
            
            FKetiSensorUtility::SaveTextureToDiskPixels(capture.Res, capture.Bitmap, pathSaveTextureTest);
#if CaptureLinearColor == 1
            FKetiSensorUtility::SavePixelsToDiskDepthTest(capture.Res, capture.BitmapLinear, pathSaveTextureTest);
#endif

            TArray<FColor> fisheyeBits;
            FKetiSensorUtility::MakeDistortFisheyeSimple(capture.Res.X, capture.Res.Y, GLidarTestFisheyeSimpleDistort,
                                                            capture.Bitmap, fisheyeBits);
            
            pathSaveTextureTest = FString::Printf(TEXT("Captured_Test_360_fisheye_%d.bmp"), icapture);
            FKetiSensorUtility::SaveTextureToDiskPixels(capture.Res, fisheyeBits, pathSaveTextureTest);
        }
        GLidarSaveCaptureTextureRequest = 0;
    }
}

void AKetiSensorRayCastLidar::TickCaptureTest(UWorld *World, const float DeltaTime)
{
    if (GLidarEmptyCastsRequest != 0)
    {
        for (int idxCapture = 0; idxCapture < AroundCaptures.Num(); ++idxCapture)
        {
            AroundCaptures[idxCapture].CollectRays.Empty();
        }

        TestCaptureRays.Empty();
        GLidarEmptyCastsRequest = 0;
    }
    
    if (GLidarLidarTestTypeAround != 0)
    {
        TickCaptureAroundRotationTest(World, DeltaTime);
    }
    else
    {
        TickCaptureMakeTextureRayTest(World, DeltaTime);   
    }
}

bool AKetiSensorRayCastLidar::GetColorFromCapture(  const FKetiSensorCapture& capture,
                                                    const FRotator& rotatorRay,
                                                    const FVector& locHit,
                                                    const TArray<FColor>& bitmap,
                                                    FColor& color_out,
                                                    FIntPoint& uv_out)
{
    FVector locHit2 = capture.SensorTMInverse.TransformPosition(locHit);
    FVector locHitProj(locHit2.Y, locHit2.Z, locHit2.X);
    locHitProj = capture.ClipProjMatrix.TransformPosition(locHitProj);
    locHitProj = locHitProj.Projection();

    float ratioX = (locHitProj.X + 1.0f) * 0.5f;
    float ratioY = 1.0f - (locHitProj.Y + 1.0f) * 0.5f;

    if (ratioX < 0 || ratioX > 1.0f) return false;
    if (ratioY < 0 || ratioY > 1.0f) return false;
    
    int x = FMath::Floor(ratioX * (float)(capture.Res.X - 1));
    int y = FMath::Floor(ratioY * (capture.Res.Y - 1));

    color_out = bitmap[capture.RenderTarget->SizeX * y + x];
    uv_out.X = x; uv_out.Y = y;

    return true;
}

bool AKetiSensorRayCastLidar::GetColorCaptured( const FKetiSensorCapture& capture,
                                                        const FRotator& rotatorRay,
                                                        const FVector& locHit,
                                                        const TArray<FColor>& bitmap,
                                                        FColor& color_out,
                                                        FIntPoint& uv_out)
{
    return GetColorFromCapture(capture, rotatorRay, locHit, bitmap, color_out, uv_out);
}

bool AKetiSensorRayCastLidar::ArrangeRayToAroundCapture(FCaptureRay& cast, TArray<FKetiSensorCapture>& aroundCaptures, int32& indexpack_out)
{
    FKetiSensorCapture* capture = aroundCaptures.FindByPredicate(
            [&](FKetiSensorCapture& iter)
                { return iter.IsAngleYawRange(cast.Rot.Yaw); });
    ensure(capture);
    if (capture == nullptr) return false;
    
    capture->ArrangeRay(cast, indexpack_out);
    return true;
}

FCaptureRay* AKetiSensorRayCastLidar::ArrangeRayToAroundCaptureNew( FHitResult& hit, FTransform& tmSensor,
                                                                    TArray<FKetiSensorCapture>& aroundCaptures,
                                                                    int32& indexpack_out )
{
    FRotator rotLaser = (hit.TraceEnd - hit.TraceStart).Rotation();
    FKetiSensorCapture* capture = aroundCaptures.FindByPredicate(
            [&](FKetiSensorCapture& iter)
                { return iter.IsAngleYawRange(rotLaser.Yaw); });
    ensure(capture);
    if (capture == nullptr) return nullptr;
    
    return capture->GetArrangedRay(hit, indexpack_out);
}

void AKetiSensorRayCastLidar::TickCaptureTextureRayTest(UWorld* World, const float DeltaTime)
{
    TArray<FCaptureRay> collectRays;
    FColor colorSemantic;
    FIntPoint uv;
    for (int iChannel = 0; iChannel < LidarRecordedHits.size(); ++iChannel)
    {
        std::vector<FHitResult>& hits = LidarRecordedHits[iChannel];
        for (int iHit = 0; iHit < hits.size(); ++iHit)
        {
            FHitResult& hit = hits[iHit];
            if (hit.bBlockingHit == false) continue;

            const AActor* actor = hit.GetActor();
            uint32_t object_tag = hit.Component != nullptr ? static_cast<uint32_t>(hit.Component->CustomDepthStencilValue) : 0;
                
            //GLog->Logf(TEXT(" object tag num :%d,[%s] ** actor name:%s"), object_tag, *SemanticNameInvs[object_tag], *actor->GetName());
            const uint8_t* colorSemanticPalette = carla::image::detail::CITYSCAPES_PALETTE_MAP[object_tag];
            colorSemantic.R = colorSemanticPalette[0];
            colorSemantic.G = colorSemanticPalette[1];
            colorSemantic.B = colorSemanticPalette[2];
            colorSemantic.A = 255;
            
            collectRays.Add(FCaptureRay(hit.TraceStart, hit.ImpactPoint, (hit.ImpactPoint - hit.TraceStart).ToOrientationRotator(),
                FColor(), colorSemantic, uv));
        }
    }

    if (collectRays.Num()<= 0) return;

    for (FKetiSensorCapture& aroundCapture : AroundCaptures)
    {
        aroundCapture.CollectRays.Empty();
    }

    //@ CaptureScene
    for (int idxCapture = 0; idxCapture < AroundCaptures.Num(); ++idxCapture)
    {
        FKetiSensorCapture& capture = AroundCaptures[idxCapture];
        if (capture.CollectRays.Num() <= 0) continue;
        
        capture.OnTickInitializeCapture();
#if CaptureLinearColor == 1
        capture.CaptureRequestWithLinear();
#else
        capture.CaptureRequest();
#endif
    }
    //@ CaptureScene //

    //@ Arrange Ray to Capture
    int indexpack = 0;
    for (int i = 0; i < collectRays.Num(); ++i)
    {
        FCaptureRay& ray = collectRays[i];
        ArrangeRayToAroundCapture(ray, AroundCaptures, indexpack);
    }
    //@ Arrange Ray to Capture //

    //@ Pick Bitmap Color
    for (int idxCapture = 0; idxCapture < AroundCaptures.Num(); ++idxCapture)
    {
        FKetiSensorCapture& capture = AroundCaptures[idxCapture];
        if (capture.CollectRays.Num() <= 0) continue;
        
        for (int i = 0; i < capture.CollectRays.Num(); ++i)
        {
            FCaptureRay& ray = capture.CollectRays[i];
            bool isGetColor = capture.GetColorFromCapture( ray.Rot, ray.Point, capture.Bitmap, ray.Color, uv);
        }
    } // for (int idxCapture = 0; idxCapture < AroundCaptures.Num(); ++idxCapture)
    //@ Pick Bitmap Color //
}

void AKetiSensorRayCastLidar::TickCaptureMakeTextureRayTest(UWorld* World, const float DeltaTime)
{
    if (CarlaHUD == nullptr) return;
    
    if (GLidarLidarShowCaptureTexture == 0)
    {
        CarlaHUD->ClearAllHUDTextureDebugDraw();
    }
    else
    {
        CarlaHUD->AddUniqueHUDTextureDraw(TestCaptureSingle.RenderTarget);
    }   
    
    FRotator rotatorSensor = GetActorRotation();
    if (GLidarCaptureRequest != 0)
    {
        TestCaptureRays.Empty();
        TestCaptureSingle.OnTickInitializeCapture();

#if CaptureLinearColor == 1
        TestCaptureSingle.CaptureRequestWithLinear();
#else
        TestCaptureSingle.CaptureRequest();
#endif

        TArray<FColor> bitmap01, bitmap03;
        //TArray<FColor> bitmap02;
        int textureU = TestCaptureSingle.RenderTarget->GetSurfaceWidth();
        int textureV = TestCaptureSingle.RenderTarget->GetSurfaceHeight();
        FIntPoint res(textureU, textureV);
        //@ 01 Render Target Texture
        UTexture2D* textureTest01 = FKetiSensorUtility::MakeTextureBitmap(textureU, textureV, bitmap01, this);
        CarlaHUD->SetOrAddHUDTextureDraw(1, textureTest01);

        FString pathSaveTexturePickTest01 = FString::Printf(TEXT("Captured_Test01_%d.bmp"), FPlatformTime::Cycles());
        FKetiSensorUtility::SaveTextureToDiskPixels(res, bitmap01, pathSaveTexturePickTest01);
        //@ 01 Render Target Texture //
        
        //@ 02 PickColor Interation Test
        FHitResult hit;
        const float multiplier = 2;
        int yawRange = FKetiSensorCommon::CaptureFOVAngleUnit;
        int pitchRange = (int)(FKetiSensorCommon::CaptureFOVAngleUnit *
                        ((float)FKetiSensorCommon::CaptureRes360.Y / (float)FKetiSensorCommon::CaptureRes360.X));
        res.X = yawRange * multiplier; res.Y = pitchRange * multiplier;
        bitmap03.SetNumZeroed(res.X * res.Y);
        
        TArray<float> AccuAngles;
        TArray<int> AccuRes;
        for (int y = 0; y < res.Y; ++y)
        {
            float currPitch = (TestCaptureSingle.AnglePitch - (float)(pitchRange / 2)) + ((float)y * (pitchRange / (float)res.Y));   
            for (int x = 0; x < res.X; ++x)
            {
                float currYaw = (TestCaptureSingle.AngleYaw - (float)(yawRange / 2)) + ((float)x * (yawRange / (float)res.X));
                FColor colorCapture, colorSemantic;
                FIntPoint uv;
                
                const float VertAngle = currPitch;
                const float HorizAngle = currYaw;

                bool isCast = ShootLaserKeti(VertAngle, HorizAngle, hit, TraceParams); 
                if (isCast == false) continue; 
                FVector dirRay = hit.TraceEnd - hit.TraceStart;
                FRotator rotRay = dirRay.Rotation();
                FVector locHit = hit.ImpactPoint;
                bool isGetColor = TestCaptureSingle.GetColorFromCapture(rotRay, locHit, bitmap01, colorCapture, uv);
                if (isGetColor == false)
                {
                    continue;
                }
                const AActor* actor = hit.GetActor();
                uint32_t object_tag = hit.Component != nullptr ? static_cast<uint32_t>(hit.Component->CustomDepthStencilValue) : 0;
                colorSemantic = FCaptureRay::GetColorSemanticTag(object_tag);

                TestCaptureRays.Add(FCaptureRay(hit.TraceStart, locHit, rotRay, colorCapture, colorSemantic, uv));
                
                if (y == 0)
                {
                    AccuAngles.Add(currYaw);
                    AccuRes.Add(uv.X);
                }
            }
        } // for (int y = 0; y < res.Y; ++y)
        
        UTexture2D* textureTest03 = FKetiSensorUtility::MakeTextureBitmap(res.X, res.Y, bitmap03, this);
        CarlaHUD->SetOrAddHUDTextureDraw(3, textureTest03);

        FString pathSaveTexturePickTest03 = FString::Printf(TEXT("Captured_Test03_PickColor_%d.bmp"), FPlatformTime::Cycles());
        FKetiSensorUtility::SaveTextureToDiskPixels(res, bitmap03, pathSaveTexturePickTest03);
        //@ 02 PickColor Interation Test //
        
        GLidarCaptureRequest = 0;
    }
}

void AKetiSensorRayCastLidar::TickCaptureAroundRotationTest(UWorld* World, const float DeltaTime)
{
    if (CarlaHUD == nullptr) return;
    
    if (GLidarLidarShowCaptureTexture == 0)
    {
        CarlaHUD->ClearAllHUDTextureDebugDraw();
    }
    else
    {
        CarlaHUD->AddUniqueHUDTextureDraw(TestCaptureSingle.RenderTarget);
    }   
    
    FTransform tmSensor = GetTransform();

    if (GLidarCaptureRequest != 0)
    {
        SimulateLidar(10.0f);

        //@ 1. Capture Scene to Render Texture
        for (int i = 0; i < AroundCaptures.Num(); ++i)
        {
            FKetiSensorCapture& capture = AroundCaptures[i]; 
            capture.CollectRays.Empty();
            
            capture.OnTickInitializeCapture();
            capture.CaptureRequestWithLinear();            
        
            int textureU = capture.RenderTarget->GetSurfaceWidth();
            int textureV = capture.RenderTarget->GetSurfaceHeight();
            FIntPoint res(textureU, textureV);
            UTexture2D* textureTest01 = FKetiSensorUtility::MakeTextureBitmap(textureU, textureV, capture.Bitmap, this);
            CarlaHUD->SetOrAddHUDTextureDraw(i + 1, textureTest01);

            FString pathSaveTexturePickTest01 = FString::Printf(TEXT("Captured_Test01_%d.bmp"), FPlatformTime::Cycles());
            FKetiSensorUtility::SaveTextureToDiskPixels(res, capture.Bitmap, pathSaveTexturePickTest01);
        } // for (int i = 0; i < AroundCaptures.Num(); ++i)
        //@ 1. Capture Scene to Render Texture //
        
        //@ 2. Arrange ray to each Capture side
        TestCaptureSingle.CollectRays.Empty();
        ParallelFor(LidarRecordedHits.size(), [&](int32 iChannel ) {
            std::vector<FHitResult>& hits = LidarRecordedHits[iChannel];
            for (int iHit = 0; iHit < hits.size(); ++iHit)
            {
                FHitResult& hit = hits[iHit];
                if (hit.bBlockingHit == false) continue;
                FCaptureRay captureRay(hit, tmSensor);
                int32 indexpack = 0;
                ArrangeRayToAroundCapture(captureRay, AroundCaptures, indexpack);
                hit.Item = indexpack;
            }
        });
        //@ 2. Arrange ray to each Capture side // 

        //@ 3. Pick Color from bitmap array.
        for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
        {
            FKetiSensorCapture& capture = AroundCaptures[icapture];
            for (int iray = 0; iray < capture.CollectRays.Num(); ++iray)
            {
                FCaptureRay& rayHit = capture.CollectRays[iray];
                rayHit.bPicked = capture.GetColorFromCapture( rayHit.Rot, rayHit.Point, capture.Bitmap, rayHit.Color, rayHit.UV);
            }   
        }
        //@ 3. Pick Color from bitmap array. //
        
        GLidarCaptureRequest = 0;
    } // if (GLidarCaptureRequest != 0)
}

void AKetiSensorRayCastLidar::TickCaptureAroundRotation(UWorld* World, const float DeltaTime)
{
    FProfiler::Begin(20);
    //@ 0. Ready capture.
    int cntRayCapture = (int)((float)((ActiveDescription.Channels) * ActiveDescription.ResolutionHorizontal) / FMath::Max((AroundCaptures.Num() - 1), 1));  
    for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
    {
        FKetiSensorCapture& capture = AroundCaptures[icapture];
        capture.CollectRays.SetNum(cntRayCapture);
        capture.IteratorIdxForArrangeRay = 0;
        capture.OnTickInitializeCapture();
    }
    //@ 0. Ready capture. //
    FProfiler::End(20);

    FProfiler::Begin(21);
    //@ 1. Collect Ray
    FTransform tmSensor = GetTransform();
    //FQuat quat(GetActorUpVector(), 90.0f * (3.141592653589f / 180.0f));
    FQuat quat(GetActorUpVector(), 0);
    tmSensor.ConcatenateRotation(quat);
    for (int iChannel = 0; iChannel < LidarRecordedHits.size(); ++iChannel)
    {
        std::vector<FHitResult>& hits = LidarRecordedHits[iChannel];
        for (int iHit = 0; iHit < hits.size(); ++iHit)
        {
            FHitResult& hit = hits[iHit];
            if (hit.bBlockingHit == false) continue;
            int32 indexpack = 0;
            FCaptureRay* captureRay = ArrangeRayToAroundCaptureNew(hit, tmSensor, AroundCaptures, indexpack);
            captureRay->InitializeCaptureRay(hit, tmSensor);
            hit.Item = indexpack;
        }
    } // for (int iChannel = 0; iChannel < LidarRecordedHits.size(); ++iChannel)
    //@ 1. Collect Ray //
    FProfiler::End(21);

    FProfiler::Begin(22);
    //@ 2. Capture Scene Render Texture
    for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
    {
        FKetiSensorCapture& capture = AroundCaptures[icapture];
        if (capture.CollectRays.Num() <= 0) continue;

        capture.CaptureRequestWithLinear();
    } // for (int i = 0; i < AroundCaptures.Num(); ++i)
    //@ 2. Capture Scene Render Texture //
    FProfiler::End(22);
}

void AKetiSensorRayCastLidar::TickVisualizeByLineBatcherRTRay(UWorld* World, const float DeltaTime)
{
    TArray<FLinearColor>& depthLinearColors = RTRayRunner->DepthLinearColors;
    TArray<FColor>& depthColors = RTRayRunner->DepthColors;
    TArray<float>& depthDists = RTRayRunner->RTRayHits;
    const FIntPoint& depthRes = RTRayRunner->ResDepthBuffer;
    const uint32 numRay = depthRes.X * depthRes.Y;
    const FTransform3f& tmSensor = FTransform3f(GetTransform());
    const FTransform3f& tmInvSensor = tmSensor.Inverse();
    const FVector3f posSensor = tmSensor.GetLocation();
    const FRotator3f rotSensor = tmSensor.Rotator();
    const FTransform3f tmCamera = FTransform3f(RTRayRunner->CameraManager->GetTransform());
    const FTransform3f tmInvCamera = FTransform3f(tmCamera.Inverse());
    const FTransform3f tmRelativeSensor = tmCamera * tmInvSensor;
    FRotator3f rotRelativeSensor = tmRelativeSensor.Rotator();
    const float distMaxRay = RTRayRunner->FarClipView;
    ensure(depthLinearColors.Num() == numRay && depthColors.Num() == numRay);
    rotRelativeSensor.Roll = rotRelativeSensor.Pitch = 0; 

    ULineBatchComponent* lineBatcher = World->LineBatcher;
    FVector3d posRaycastd;
    for (int v = 0; v < depthRes.Y; ++v)
    {
        for (int h = 0; h < depthRes.X; ++h)
        {
            int idxInv = (depthRes.Y - v - 1) * depthRes.X + h;
            int idx = v * depthRes.X + h;
            FLinearColor& lcolor = depthLinearColors[idxInv];
            FColor& color = depthColors[idxInv];
            const FVector3f& rtLaser = rotRelativeSensor.RotateVector(RTRayRunner->GetLaserRay(v, h));
            //rtLaser = FVector3f(rtLaser.X, rtLaser.Y, rtLaser.Z);
            //FVector3f rtLaser = tmInvSensor.TransformVector(rtLasers[idxInv]);
            //FVector3f rtLaser = rotCamera.RotateVector(rtLasers[idxInv]);
            //FVector3f rtLaser = tmInvCamera.TransformVector(rtLasers[idxInv]);
            //float distancel = ((lcolor.R + lcolor.G + lcolor.B) / 3.0f) * distMaxRay;
            //float distance = ((float)(color.R + color.G + color.B + color.A) / (4.0f
            //    * 255.0f)) * distMaxRay;
            float distance = depthDists[idx];
            
            //float distance = ((float)color.B / 255.0f) * distMaxRay;
            //float distance = lcolor.B * distMaxRay;
            if (distance <= KINDA_SMALL_NUMBER) continue;
            
            FVector3f posCast = posSensor + (rtLaser * distance);
            posRaycastd.Set(posCast.X, posCast.Y, posCast.Z);
            lineBatcher->DrawPoint(posRaycastd, FColor::Cyan, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
        }
    }
}

void AKetiSensorRayCastLidar::TickVisualizeByLineBatcher(UWorld* World, const float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_LidarTickVisualizeByLineBatcher);

    ULineBatchComponent* LineBatcher = World->LineBatcher;

    APlayerController *pc = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ensure(pc);
    ensure(pc->PlayerCameraManager);
    FVector posCamera = pc->PlayerCameraManager->GetCameraLocation();
    
    if (GLidarTestRaycastDrawDirectly == 1)
    {
        for (int i = 0; i < LidarRecordedHits.size(); ++i)
        {
            std::vector<FHitResult>& hits = LidarRecordedHits[i];
            for (int j = 0; j < hits.size(); ++j)
            {
                FHitResult& hit = hits[j];
                LineBatcher->DrawPoint(hit.ImpactPoint, FColor::Cyan, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
            }
        }
        return;
    }
    
    if (GLidarLidarTest == 1 && TestCaptureRays.Num() >= 1)
    {
        if (GLidarShowDrawLines != 0)
        {
            FVector locStartCapture = TestCaptureSingle.CaptureComp->GetComponentLocation();
            FVector locEndCapture = locStartCapture + (TestCaptureSingle.CaptureComp->GetForwardVector() * 500);
            LineBatcher->DrawPoint(locStartCapture, FLinearColor::Blue, 20.0f, SDPG_Foreground);
            LineBatcher->DrawLine(locStartCapture, locEndCapture, FLinearColor::Red, SDPG_Foreground, 2.0f, ActiveDescription.SizeBaseDots);
            
            for (int i = 1; i < TestCaptureRays.Num(); ++i)
            {
                LineBatcher->DrawLine(TestCaptureRays[i].From, TestCaptureRays[i].Point, FLinearColor(1,1,0, 0.5f), SDPG_Foreground, 1.0f, ActiveDescription.SizeBaseDots);   
            }   
        }

        if (ActiveDescription.bVisualizeIntensity == 1)
        {
            for (int i = 0; i < TestCaptureRays.Num(); ++i)
            {
                FCaptureRay& cast = TestCaptureRays[i];
                LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
            }
        }
        else
        {
            if (GLidarVisualizeSemantic == 0)
            {
                for (int i = 0; i < TestCaptureRays.Num(); ++i)
                {
                    FCaptureRay& cast = TestCaptureRays[i];
                    LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                }   
            }
            else
            {
                for (int i = 0; i < TestCaptureRays.Num(); ++i)
                {
                    FCaptureRay& cast = TestCaptureRays[i];
                    LineBatcher->DrawPoint(cast.Point, cast.ColorSemantic, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                }
            }   
        }
    }
    else if (GLidarLidarTest != 1)
    {
        if (GLidarShowDrawLines != 0)
        {
            for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
            {
                FKetiSensorCapture& capture         = AroundCaptures[icapture];
                FVector       locStartCapture = capture.CaptureComp->GetComponentLocation();
                FVector       locEndCapture   = locStartCapture + (capture.CaptureComp->GetForwardVector() * 500);
                LineBatcher->DrawPoint(locStartCapture, FLinearColor::Blue, ActiveDescription.SizeBaseDots, SDPG_Foreground);
                LineBatcher->DrawLine(locStartCapture, locEndCapture, FLinearColor::Red, SDPG_Foreground, 1.0f, ActiveDescription.SizeBaseDots);
            }
        
            for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
            {
                FKetiSensorCapture& capture = AroundCaptures[icapture];
                for (int i = 0; i < capture.CollectRays.Num(); ++i)
                {
                    FCaptureRay& ray = capture.CollectRays[i];
                    LineBatcher->DrawLine(ray.From, ray.Point, FLinearColor(1,1,0, 0.5f), SDPG_Foreground, 1.0f, ActiveDescription.SizeBaseDots);
                }
            }
        }
        
        if (GLidarVisualize360 == 1)
        {
            FTransform tmSensorForCapture = GetTransform();
            FRotator rotSensor = tmSensorForCapture.Rotator();
            rotSensor.Yaw += 90.0f;
            tmSensorForCapture.SetRotation(rotSensor.Quaternion());
            FTransform invtm = tmSensorForCapture.Inverse();
            
            FTransform tmSensor = GetTransform();
            //FRotator rotSensorTrack = rotSensor;
            //rotSensorTrack.Pitch = rotSensorTrack.Roll = 0;
            //tmSensor.SetRotation(rotSensorTrack.Quaternion());

            if (GLidarVisualize360Sonar == 1)
            {
                for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                {
                    FKetiSensorCapture& capture = AroundCaptures[icapture];
                    for (int i = 0; i < capture.CollectRays.Num(); ++i)
                    {
                        FCaptureRay& ray = capture.CollectRays[i];
                        if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                        IndexRing++;
                        IndexRing = IndexRing >= CountTotalLaser ? IndexRing - CountTotalLaser : IndexRing;  
                        DrawCircleRingRays[IndexRing] = ray;
                        DrawUIRayRingPoints[IndexRing] = FVector2D(invtm.TransformPosition(ray.Point));
                        DrawUIRayRingColors[IndexRing] = ActiveDescription.bVisualizeIntensity == true ? ray.ColorIntensity : ray.Color;
                    }
                }

                if (GVisualizeTrackToVehicle == 1 && DrawCircleRingRays.Num() >= 1)
                {
                    for (int i = 0; i < DrawCircleRingRays.Num(); ++i)
                    {
                        DrawCircleRingRays[i].UpdateLocalToWorldSonar(TMSensorTracks,
                                            AngleHAbsCache, TimeNowOnTick, ActiveDescription.RotationFrequency);
                    }
                }
    
                if (ActiveDescription.bVisualizeIntensity == true)
                {
                    for (int i = 0; i < DrawCircleRingRays.Num(); ++i)
                    {
                        FCaptureRay& cast = DrawCircleRingRays[i];
                        float angleRay = cast.Hit.Time;
                        if (AngleHAbsCache - angleRay > 1.0) continue;
                        
                        //if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                        LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                        //LineBatcher->DrawPoint(ray.hit.ImpactPoint, ray.ColorIntensity, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                    }
                }
                else
                {
                    for (int i = 0; i < DrawCircleRingRays.Num(); ++i)
                    {
                        FCaptureRay& cast = DrawCircleRingRays[i];
                        float angleRay = cast.Hit.Time;
                        if (AngleHAbsCache - angleRay > 1.0) continue;
                        
                        //if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                        LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                        //LineBatcher->DrawPoint(ray.hit.ImpactPoint, ray.Color, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                    }
                }
            }
            else
            {
                FProfiler::Begin(41);
                int index360 = 0;
                for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                {
                    FKetiSensorCapture& capture = AroundCaptures[icapture];
                    for (int i = 0; i < capture.CollectRays.Num(); ++i)
                    {
                        FCaptureRay& ray = capture.CollectRays[i];
                        if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                        
                        if (Index360Iterator >= Index360Capacity) break;
                        index360 = Index360Iterator++;
                        ray.UpdateLocalToWorld(tmSensor);
                        DrawCircleRays[index360] = ray;
                        DrawUIRayPoints[index360] = FVector2D(invtm.TransformPosition(ray.Point));
                        //DrawUIRayPoints[index360] = FVector2D(ray.PointLocal);
                        DrawUIRayColors[index360] = ActiveDescription.bVisualizeIntensity == true ? ray.ColorIntensity : ray.Color;
                    }
                    if (Index360Iterator >= Index360Capacity) break;
                }
                FProfiler::End(41);
                
                if ((AngleDraw360Cache - AngleDraw360CacheStart) > 360.0f)
                {
                    FProfiler::Begin(42);
                    //DrawCircleRays = CacheDrawCircleRays;
                    //DrawUIRayPoints = CacheDrawUIRayPoints;
                    //DrawUIRayColors = CacheDrawUIRayColors;
            
                    AngleDraw360Cache = AngleDraw360CacheStart = AngleHorizontalCurrent;
                    Index360Iterator = 0;
                    TMSensorAroundViewLast = tmSensor;
                    FProfiler::End(42);
                }
                
                // if (GVisualizeTrackToVehicle == 1 && DrawCircleRays.Num() >= 1)
                // {
                //     FProfiler::Begin(43);
                //     if (GLidarDistortionLerp == 1)
                //     {
                //         for (int i = 0; i < DrawCircleRays.Num(); ++i)
                //         {
                //             DrawCircleRays[i].UpdateLocalToWorldLerpOnTrack(TMSensorTracks, TimeNowOnTick);
                //         }
                //     }
                //     else
                //     {
                //         for (int i = 0; i < DrawCircleRays.Num(); ++i)
                //         {
                //             DrawCircleRays[i].UpdateLocalToWorld(tmSensor);
                //         }
                //     }
                //     FProfiler::End(43);
                // }

                if (GLidarShowVisualizeGameWorld == 1)
                {
                    if (ActiveDescription.bVisualizeIntensity == true)
                    {
                        FProfiler::Begin(44);
                        for (int i = 0; i < DrawCircleRays.Num(); ++i)
                        {
                            FCaptureRay& cast = DrawCircleRays[i];
                            //if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                            LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                            //LineBatcher->DrawPoint(ray.hit.ImpactPoint, ray.ColorIntensity, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                        }
                        FProfiler::End(44);
                    }
                    else
                    {
                        for (int i = 0; i < DrawCircleRays.Num(); ++i)
                        {
                            FCaptureRay& cast = DrawCircleRays[i];
                            //if (ray.bPicked == false || ray.bIntensityDetected == false) continue;
                            LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                            //LineBatcher->DrawPoint(ray.hit.ImpactPoint, ray.Color, ActiveDescription.SizeBaseDots, FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);
                        }
                    }   
                }
            }
        }
        else
        {
            if (ActiveDescription.bVisualizeIntensity == true)
            {
                for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                {
                    FKetiSensorCapture& capture = AroundCaptures[icapture];
                    for (int i = 0; i < capture.CollectRays.Num(); ++i)
                    {
                        FCaptureRay& cast = capture.CollectRays[i];
                        if (cast.bPicked == false || cast.bIntensityDetected == false) continue;
                        LineBatcher->DrawPoint(cast.Point, cast.ColorIntensity, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                    }
                }
            }
            else
            {
                if (GLidarVisualizeSemantic == 0)
                {
                    for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                    {
                        FKetiSensorCapture& capture = AroundCaptures[icapture];
                        for (int i = 0; i < capture.CollectRays.Num(); ++i)
                        {
                            FCaptureRay& cast = capture.CollectRays[i];
                            if (cast.bPicked == false) continue;
                            LineBatcher->DrawPoint(cast.Point, cast.Color, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                        }
                    }
                }
                else
                {
                    for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
                    {
                        FKetiSensorCapture& capture = AroundCaptures[icapture];
                        for (int i = 0; i < capture.CollectRays.Num(); ++i)
                        {
                            FCaptureRay& cast = capture.CollectRays[i];
                            if (cast.bPicked == false) continue;
                            LineBatcher->DrawPoint(cast.Point, cast.ColorSemantic, cast.GetSizeDot(posCamera, DistanceDotSt, ActiveDescription.SizeBaseDots), FKetiSensorUtility::DepthPriorityDraw, SecondDrawDefault);   
                        }
                    }
                }   
            }   
        }
    }
}

void AKetiSensorRayCastLidar::TickVisualizeByInstancedMesh(UWorld* World, const float DeltaTime)
{    
    if (GLidarVisualize360 == 1)
    {
        FTransform tm = GetTransform();
        FRotator rot = tm.Rotator();
        rot.Yaw += 90.0f;
        tm.SetRotation(rot.Quaternion());
        FTransform invtm = tm.Inverse();
        FVector loc = tm.GetLocation();
        int index360 = 0;
        for (int icapture = 0; icapture < AroundCaptures.Num(); ++icapture)
        {
            FKetiSensorCapture& capture = AroundCaptures[icapture];
            for (int i = 0; i < capture.CollectRays.Num(); ++i)
            {
                FCaptureRay& ray = capture.CollectRays[i];
                if (ray.bPicked == false || ray.bIntensityDetected == false) continue;

                if (Index360Iterator >= Index360Capacity) break;
                index360 = Index360Iterator++;
                DrawCircleRays[index360] = ray;
                DrawUIRayPoints[index360] = FVector2D(invtm.TransformPosition(ray.Point));
                DrawUIRayColors[index360] = ActiveDescription.bVisualizeIntensity == true ? ray.ColorIntensity : ray.Color;
            }
            if (Index360Iterator >= Index360Capacity) break;
        }
            
        if ((AngleDraw360Cache - AngleDraw360CacheStart) > 360.0f)
        {            
            AngleDraw360Cache = AngleDraw360CacheStart = AngleHorizontalCurrent;
            Index360Iterator = 0;
        }

        if (DrawCircleRays.Num() >= 1 && InstancedStaticMeshComp->GetInstanceCount() != DrawCircleRays.Num())
        {
            //InstancedStaticMeshComp->ClearInstances();

            TArray<FTransform> tmDots;
            tmDots.SetNumZeroed(DrawCircleRays.Num());
            
            for (int i = 0; i < DrawCircleRays.Num(); ++i)
            {
                FTransform& tmInstance = tmDots[i]; 
                tmInstance.SetIdentity();
                FCaptureRay& ray = DrawCircleRays[i];
                tmInstance.SetTranslation(ray.Point);
            }

            int countInstanced = InstancedStaticMeshComp->GetInstanceCount();
            int countDots = DrawCircleRays.Num();
            if (countInstanced == 0)
            {
                InstancedStaticMeshComp->AddInstances(tmDots, false);
            }
            else
            {
                int cntRemain = 0;
                if (countInstanced > countDots)
                {
                    for (int i = countDots - 1; i < countInstanced; ++i)
                    {
                        InstancedStaticMeshComp->RemoveInstance(i);
                    }
                }
                else if (countInstanced < countDots)
                {
                    cntRemain = countDots - countInstanced;
                }

                for (int i = 0; i < countDots; ++i)
                {
                    InstancedStaticMeshComp->UpdateInstanceTransform(i, tmDots[i]);
                }

                if (cntRemain >= 1)
                {
                    TArray<FTransform> tmAdds;
                    tmAdds.AddZeroed(cntRemain);
                    for (int i = 0; i < cntRemain; ++i)
                    {
                        tmAdds[i] = tmDots[tmDots.Num() - cntRemain + i];
                    }
                    InstancedStaticMeshComp->AddInstances(tmAdds, false);
                }
            }
        } // if (DrawCircleRays.Num() >= 1 && InstancedStaticMeshComp->GetInstanceCount() != DrawCircleRays.Num())
    }
}

void AKetiSensorRayCastLidar::TickVisualize(UWorld* World, const float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_LidarTickVisualize);
    if (ActiveDescription.bShowVisualize == false) return;
    
    if (GLidarVisualizeByInstanced == 1)
    {
        TickVisualizeByInstancedMesh(World, DeltaTime);
    }
    else
    {
        TickVisualizeByLineBatcher(World, DeltaTime);
    }
}

void AKetiSensorRayCastLidar::SimulateLidarAround(const UWorld *World, const float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_LidarTickSimulateLidar);

    FProfiler::CountingRay = FProfiler::CountingRaycast = 0;
    
	const uint32 ChannelCount = ActiveDescription.Channels;
	const uint32 CountHScanALaser =
	FMath::RoundHalfFromZero(
		(ActiveDescription.RotationFrequency * ActiveDescription.HorizontalFov) * DeltaTime) / AngleUnitHorizontalLidar;
	
	if (CountHScanALaser <= 0)
	{
	    GLog->Logf(TEXT("%s: no points requested this frame, try increasing the number of points per second."), *GetName());
	    return;
	}
	
	check(ChannelCount == LaserAngles.Num());
	
	const float CurrentHorizontalAngle = AngleHorizontalCurrent;
	const float AngleHoriDistUnitLaser = AngleUnitHorizontalLidar;
    const float AngleHoriFOV           = ActiveDescription.HorizontalFov;
    const float AngleHoriFOVSkipRange  = 360.0f - AngleHoriFOV;
    const float AngleHoriFOVSkipStart  = AngleHoriFOV * 0.5f;
	
	ResetLidarRecordedHits(ChannelCount, CountHScanALaser);
	PreprocessRays(ChannelCount, CountHScanALaser);

    int countActualScan = 0;

    auto LockedPhysObject = FPhysicsObjectExternalInterface::LockRead(GetWorld()->GetPhysicsScene());
	ParallelFor(ChannelCount, [&](int32 idxChannel ) {
		for (auto idxPtsOneLaser = 0u; idxPtsOneLaser < CountHScanALaser; idxPtsOneLaser++)
        {
		    FHitResult HitResult;
		    const float VertAngle = LaserAngles[idxChannel];
		    const float HorizAngleRelative = AngleHoriDistUnitLaser * idxPtsOneLaser;
            const float HorizAngle = std::fmod(CurrentHorizontalAngle + HorizAngleRelative, 360.0f);
            if (AngleHoriFOV < (360.0f - 0.1f) 
                && (HorizAngle >= AngleHoriFOVSkipStart && HorizAngle <= (AngleHoriFOVSkipStart + AngleHoriFOVSkipRange)))
            {
                continue;
            }

		    const bool PreprocessResult = RayPreprocessCondition[idxChannel][idxPtsOneLaser];

            FProfiler::CountingRay++;
		    if (PreprocessResult && ShootLaserKeti(VertAngle, HorizAngle, HitResult, TraceParams))
            {
		        FProfiler::CountingRaycast++;

		        //@ distortion 01 
		        float ratioYaw = HorizAngle / 360.0f;
		        HitResult.FaceIndex = (int32)((double)MaxInt * ratioYaw);
		        //@ distortion 02
		        double angleHAbsCurrent = (AngleHAbsCache + (HorizAngleRelative / 360.0));
		        HitResult.Time = angleHAbsCurrent;
		        
			    WritePointAsyncKeti(idxChannel, HitResult);
		        countActualScan++;
		    }
		}
	});
    LockedPhysObject.Release();

    CountAroundRing = (CountAroundRing + countActualScan) % CountTotalLaser; 
    
    const float AngleDistanceOfTick = AngleUnitHorizontalLidar * (CountHScanALaser);
    FRotator laserRotYaw(0, CurrentHorizontalAngle, 0);
	const float HorizontalAngle = std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f);
	KetiLidarData.SetHorizontalAngle(HorizontalAngle);
    AngleHorizontalCurrent = HorizontalAngle;
    AngleDraw360Cache += AngleDistanceOfTick;
    AngleHAbsCache += (AngleDistanceOfTick / 360.0f);
}

void AKetiSensorRayCastLidar::SimulateLidarAroundRTRayBuffer(const UWorld* World, const float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_LidarTickSimulateLidar);

    FProfiler::CountingRay = FProfiler::CountingRaycast = 0;
    
	const uint32 ChannelCount = ActiveDescription.Channels;
	const uint32 CountHScanALaser =
	    FMath::Clamp(
	    FMath::RoundHalfFromZero(
		    (ActiveDescription.RotationFrequency * ActiveDescription.HorizontalFov) * DeltaTime) / AngleUnitHorizontalLidar
		    , 0, ActiveDescription.ResolutionHorizontal);
	
	if (CountHScanALaser <= 0)
	{
	    GLog->Logf(TEXT("%s: no points requested this frame, try increasing the number of points per second."), *GetName());
	    return;
	}
	
	const float CurrentHorizontalAngle = AngleHorizontalCurrent;
	const float AngleHoriDistUnitLaser = AngleUnitHorizontalLidar;
    const float AngleHoriFOV           = ActiveDescription.HorizontalFov;
    const float AngleHoriFOVSkipRange  = 360.0f - AngleHoriFOV;
    const float AngleHoriFOVSkipStart  = AngleHoriFOV * 0.5f;
    const FVector LocationSensor       = GetActorLocation();
	
	ResetLidarRecordedHits(ChannelCount, CountHScanALaser);
	PreprocessRays(ChannelCount, CountHScanALaser);

    int countActualScan = 0;
    
	ParallelFor(ChannelCount, [&](int32 idxChannel ) {
	//for (uint32 idxChannel = 0; idxChannel < ChannelCount; ++idxChannel)
    //{
        for (auto idxPtsOneLaser = 0u; idxPtsOneLaser < CountHScanALaser; idxPtsOneLaser++)
        {
            FHitResult HitResult;
            const float VertAngle = LaserAngles[idxChannel];
            const float HorizAngleRelative = AngleHoriDistUnitLaser * idxPtsOneLaser;
            const float HorizAngle = std::fmod(CurrentHorizontalAngle + HorizAngleRelative, 360.0f);
            //if (HorizAngle >= AngleHoriFOVSkipStart && HorizAngle <= (AngleHoriFOVSkipStart + AngleHoriFOVSkipRange))
            //{
            //    continue;
            //}

            const bool PreprocessResult = RayPreprocessCondition[idxChannel][idxPtsOneLaser];
            const int hIdx = (IndexLaserRTRayBuffer + idxPtsOneLaser) % ActiveDescription.ResolutionHorizontal;

            FProfiler::CountingRay++;
            if (PreprocessResult && ShootLaserKetiRTRayBuffer(idxChannel, hIdx, VertAngle, HorizAngle, LocationSensor, HitResult))
            {
                FProfiler::CountingRaycast++;

                //@ distortion 01 
                float ratioYaw = HorizAngle / 360.0f;
                HitResult.FaceIndex = (int32)((double)MaxInt * ratioYaw);
                //@ distortion 02
                double angleHAbsCurrent = (AngleHAbsCache + (HorizAngleRelative / 360.0));
                HitResult.Time = angleHAbsCurrent;
		    
                WritePointAsyncKeti(idxChannel, HitResult);
                countActualScan++;
            }
        }
    });
    
    IndexLaserRTRayBuffer = (IndexLaserRTRayBuffer + CountHScanALaser) % ActiveDescription.ResolutionHorizontal;
    CountLaserRTRayBuffer = (CountLaserRTRayBuffer + countActualScan) % CountTotalLaser; 
    
    const float AngleDistanceOfTick = AngleUnitHorizontalLidar * (CountHScanALaser);
    FRotator laserRotYaw(0, CurrentHorizontalAngle, 0);
	AngleHorizontalCurrent = std::fmod(CurrentHorizontalAngle + AngleDistanceOfTick, 360.0f);
	KetiLidarData.SetHorizontalAngle(AngleHorizontalCurrent);
    AngleDraw360Cache += AngleDistanceOfTick;
    AngleHAbsCache += (AngleDistanceOfTick / 360.0f);   
}

bool AKetiSensorRayCastLidar::ShootLaserKeti(   const FTransform& tmSensor,
                                                const float VerticalAngle,
                                                float HorizontalAngle,
                                                FHitResult &HitResult,
                                                FCollisionQueryParams& traceParams) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);

    FHitResult HitInfo(ForceInit);
    
    FVector LidarBodyLoc = tmSensor.GetLocation();
    FRotator LidarBodyRot = tmSensor.Rotator();

    FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
    FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
      LaserRot,
      LidarBodyRot
    );

    const float rangeMin = ActiveDescription.RangeMin;
    const float rangeMax = ActiveDescription.RangeMax;
    
    FVector StartTrace = LidarBodyLoc;
    FVector EndTrace = LidarBodyLoc + (rangeMax * UKismetMathLibrary::GetForwardVector(ResultRot));

    GetWorld()->LineTraceSingleByChannel(
      HitInfo,
      StartTrace,
      EndTrace,
      ECC_GameTraceChannel2,
      traceParams,
      FCollisionResponseParams::DefaultResponseParam
    );

    const float dist = HitInfo.Distance;
    if (dist < rangeMin)
    {
        HitInfo.bBlockingHit = false;
    }
    
    if (HitInfo.bBlockingHit) {
        HitResult = HitInfo;
        return true;
    } else {
        return false;
    }
}

bool AKetiSensorRayCastLidar::ShootLaserKeti(   const float VerticalAngle,
                                                float HorizontalAngle,
                                                FHitResult& HitResult,
                                                FCollisionQueryParams& traceParams) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
    FTransform ActorTransf = GetTransform();
    FVector LidarBodyLoc = ActorTransf.GetLocation();
    FRotator LidarBodyRot = ActorTransf.Rotator();

    FRotator LaserRot (VerticalAngle, HorizontalAngle, 0);  // float InPitch, float InYaw, float InRoll
    FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
      LaserRot,
      LidarBodyRot
    );

    const float rangeMin = ActiveDescription.RangeMin;
    const float rangeMax = ActiveDescription.RangeMax;
    
    FVector StartTrace = LidarBodyLoc;
    FVector EndTrace = LidarBodyLoc + (rangeMax * UKismetMathLibrary::GetForwardVector(ResultRot));

    FHitResult HitInfo(ForceInit);
    GetWorld()->LineTraceSingleByChannel(
      HitInfo,
      StartTrace,
      EndTrace,
      ECC_GameTraceChannel2,
      traceParams,
      FCollisionResponseParams::DefaultResponseParam
    );

    const float dist = HitInfo.Distance;
    if (dist < rangeMin)
    {
        HitInfo.bBlockingHit = false;
    }
    
    if (HitInfo.bBlockingHit) {
        HitResult = HitInfo;
        return true;
    } else {
        return false;
    }
}

bool AKetiSensorRayCastLidar::ShootLaserKetiRTRayBuffer(const uint32 idxV, const uint32 idxH,
                                                        const float vAngle, const float hAngle,
                                                        const FVector& locationSensor,
                                                        FHitResult& HitResult) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
    //FTransform ActorTransf = GetTransform();
    //FVector LidarBodyLoc = ActorTransf.GetLocation();
    //FRotator LidarBodyRot = ActorTransf.Rotator();
    //
    //FRotator LaserRot (vAngle, hAngle, 0);  // float InPitch, float InYaw, float InRoll
    //FRotator ResultRot = UKismetMathLibrary::ComposeRotators(
    //  LaserRot,
    //  LidarBodyRot
    //);

    // const float rangeMin = ActiveDescription.RangeMin;
    // const float rangeMax = ActiveDescription.RangeMax;
    //
    // FVector StartTrace = LidarBodyLoc;
    // FVector EndTrace = LidarBodyLoc + (rangeMax * UKismetMathLibrary::GetForwardVector(ResultRot));
    //FVector3f laser = FVector3f(LaserRot.Vector());
    //RTRayRunner->SetLaserRay(idxV, idxH, laser);
    const FVector3f& laser = RTRayRunner->GetLaserRay(idxV, idxH);
    float distCast = RTRayRunner->GetDistanceCastRay(idxV, idxH);
    HitResult = FHitResult();
    bool bCast = (distCast > 0);
    if (bCast == true)
    {
        HitResult.Distance = distCast;
        HitResult.bBlockingHit = true;
        HitResult.TraceStart = locationSensor;
        HitResult.ImpactPoint = HitResult.Location = locationSensor + FVector(laser * distCast);
        return true;
    }
    
    return false;
}

void AKetiSensorRayCastLidar::SimulateLidarRTRayProjectionTest(const UWorld* World, const float DeltaTime)
{
    if (RTRayRunner == nullptr) return;
    
    TArray<FColor>& bitmapDepth = RTRayRunner->DepthColors;
    FTransform tmOrigin = GetTransform();
    FVector3d origin = tmOrigin.GetLocation();
    FIntPoint resDepthBuffer = RTRayRunner->ResDepthBuffer;
    float fovHori = 90.0f;
    float fovHoriMin = -(fovHori * 0.5f);
    float fovVertMin = Description.LowerFovLimit;
    float fovVertMax = Description.UpperFovLimit;
    float fovVert = fovVertMax - fovVertMin; 
    FRotator rotActor = tmOrigin.Rotator(); 
    FRotator rotDirIterator;
    
    TestRTRayPickings.SetNum(resDepthBuffer.X * resDepthBuffer.Y);
    
    for (int v = 0; v < (int)fovVert; ++v)
    //ParallelFor(RTRayRunner->Res.Y, [&](int32 v)
    {
        float fovV = fovVertMin + (fovVert * ((float)v / (fovVert - 1)));
        for (int h = 0; h < (int)fovHori; ++h)
        {
            float fovH = fovHoriMin + (fovHori * ((float)h / (fovHori - 1)));
            rotDirIterator.Yaw = fovH;
            rotDirIterator.Pitch = fovV;
            FVector dirRay = rotDirIterator.Vector();
            //FColor color;
            //FIntPoint uv;
            //RTRayRunner->GetColorPickDepth(dirRay, color, uv);
            float distPicked = RTRayRunner->GetDistanceCastRayByProject(dirRay);
            FVector dirRayWorld = tmOrigin.TransformVector(dirRay);
            FVector pos = origin + (dirRayWorld * distPicked);
            TestRTRayPickings[v * RTRayRunner->ResDepthBuffer.X + h] = pos;
        }
    }
    //);
}

void AKetiSensorRayCastLidar::MakeOneLaserWorldDirection(const FRotator& sensorRot, const float angleV, float angleH, FVector3f& laser_out)
{
    FRotator laserRot (angleV, angleH, 0);
    laser_out = FVector3f(laserRot.Vector());
    //FRotator resultRot = UKismetMathLibrary::ComposeRotators(
    //  laserRot,
    //  sensorRot
    //);
    //laser_out = FVector3f(resultRot.Vector());
}

void AKetiSensorRayCastLidar::MakeSimulateRTRayBuffer()
{
    //const uint32 ChannelCount = ActiveDescription.Channels;
    const uint32 NumCh = RTRayRunner->ResDepthBuffer.Y;
    const uint32 numHAngle360 = RTRayRunner->ResDepthBuffer.X;	
	const float StartHorizontalAngle   = 0;
	const float AngleHoriUnit          = 360.0f / numHAngle360;
    const float AngleVertMin           = Description.LowerFovLimit;
    const float AngleVertRange         = Description.UpperFovLimit - Description.LowerFovLimit;
    const FTransform tmSensor          = GetTransform();
    const FRotator rotSensor           = tmSensor.Rotator();
    FTransform3f tmCamera              = FTransform3f(RTRayRunner->CameraManager->GetTransform());
    FTransform3f tmInvCamera           = tmCamera.Inverse();
    FTransform3f tmRelative            = tmCamera * FTransform3f(tmSensor.Inverse());
	
	//ParallelFor(NumCh, [&](int32 idxCh) {
    for (uint32 idxCh = 0; idxCh < NumCh; idxCh++)
    {
        for (uint32 idxOneLaserHori = 0; idxOneLaserHori < numHAngle360; idxOneLaserHori++)
        {
            const float VertAngle = AngleVertMin + ((float)idxCh / (float)(NumCh - 1)) * AngleVertRange;
            const float HorizAngleRelative = AngleHoriUnit * idxOneLaserHori;
            const float HorizAngle = std::fmod(StartHorizontalAngle + HorizAngleRelative, 360.0f);

            FProfiler::CountingRay++;
            FVector3f laserRay;
            MakeOneLaserWorldDirection(rotSensor, VertAngle, HorizAngle, laserRay);
            //laserRay = tmInvCamera.TransformVector(laserRay);
            //tmCamera
            //laserRay = tmRelative.TransformVector(laserRay);
            RTRayRunner->SetLaserRay(idxCh, idxOneLaserHori, laserRay);

            //GLog->Logf(TEXT("RTRayTest/idxCh(%d)/idxOneLaserHori(%d)/VertAngle(%f)/HorizAngle(%f)/laserRay(%s)"),
            //    idxCh, idxOneLaserHori, VertAngle, HorizAngle, *laserRay.ToString());
        }
    }
	//});
}

FHitResult& AKetiSensorRayCastLidar::WritePointAsyncKeti(uint32_t channel, FHitResult& detection)
{
    TRACE_CPUPROFILER_EVENT_SCOPE_STR(__FUNCTION__);
    DEBUG_ASSERT(GetChannelCount() > channel);
    FHitResult& result = LidarRecordedHits[channel].emplace_back(detection);
    return result;
}

void AKetiSensorRayCastLidar::ComputeDetectionSingleKeti(const FHitResult& HitInfo, const FTransform& SensorTransf, FDetection& detection_out) const
{
    const FVector HitPoint = HitInfo.ImpactPoint;
    detection_out.point  = SensorTransf.Inverse().TransformPosition(HitPoint);

    const float Distance = detection_out.point.Length();

    //거리에 따른 대기 감쇄
    const float AttenAtm = ActiveDescription.AtmospAttenRate;
    const float AbsAtm   = exp(-AttenAtm * Distance);

    const FVector VecInc = -(HitPoint - SensorTransf.GetLocation()).GetSafeNormal();
    const float   cosTh  = FVector::DotProduct(VecInc, HitInfo.ImpactNormal);
    
    uint32_t              object_tag = HitInfo.Component != nullptr ? static_cast<uint32_t>(HitInfo.Component->CustomDepthStencilValue) : 0;

    float matRef = Reflectivities.Contains(object_tag) ? Reflectivities[object_tag] : 0.5f;
    float colorintensity = 1.0f;
    float r, g, b, a;
    FKetiSensorCapture::GetUnpackLinearColorRGBA(HitInfo.Item, r, g, b, a);
    colorintensity = FKetiSensorUtility::GetLightColorIntensity(r, g, b);
    
    const float IntRec = ((AbsAtm * ActiveDescription.RatioIntensityAtmos) + (1.0f - ActiveDescription.RatioIntensityAtmos))
                        * ((cosTh * ActiveDescription.RatioIntensityCosin) + (1.0f - ActiveDescription.RatioIntensityCosin))
                        * ((matRef * ActiveDescription.RatioIntensityRefle) + (1.0f - ActiveDescription.RatioIntensityRefle))
                        * ((colorintensity * ActiveDescription.RatioIntensityColor) + (1.0f - ActiveDescription.RatioIntensityColor));
    detection_out.intensity = IntRec;
}

FDetection AKetiSensorRayCastLidar::ComputeDetectionSingleCarla(const FHitResult& HitInfo, const FTransform& SensorTransf) const
{
    FDetection Detection;
    const FVector HitPoint = HitInfo.ImpactPoint;
    Detection.point = SensorTransf.Inverse().TransformPosition(HitPoint);

    const float Distance = Detection.point.Length();

    const float AttenAtm = ActiveDescription.AtmospAttenRate;
    const float AbsAtm = exp(-AttenAtm * Distance);

    const float IntRec = AbsAtm;

    Detection.intensity = IntRec;
    return Detection;
}

void AKetiSensorRayCastLidar::PreprocessRays(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
    Super::PreprocessRays(Channels, MaxPointsPerChannel);

    for (auto ch = 0u; ch < Channels; ch++) {
        for (auto p = 0u; p < MaxPointsPerChannel; p++) {
            RayPreprocessCondition[ch][p] = !(DropOffGenActive && RandomEngine->GetUniformFloat() < ActiveDescription.DropOffGenRate);
        }
    }
}

bool AKetiSensorRayCastLidar::PostprocessDetection(FDetection& Detection) const
{
    if (ActiveDescription.bUseLidarNoise == true && ActiveDescription.NoiseStdDev > std::numeric_limits<float>::epsilon()) {
        const auto ForwardVector = Detection.point.MakeUnitVector();
        const auto Noise = ForwardVector * RandomEngine->GetNormalDistribution(0.0f, ActiveDescription.NoiseStdDev);
        Detection.point += Noise;
    }

    const float Intensity = Detection.intensity;
    if(Intensity > ActiveDescription.DropOffIntensityLimit)
    {
        return true;
    }
    else
    {
        return RandomEngine->GetUniformFloat() < DropOffAlpha * Intensity + DropOffBeta;
    }
}

bool AKetiSensorRayCastLidar::PostprocessDetectionForKeti(FDetection& Detection, const FHitResult& hit, float range, FVector& noise_out)
{
    // Tick Noise 
    if (ActiveDescription.bUseLidarNoise == true)
    {
        float distance = hit.Distance;
        const float dist = FMath::Clamp(distance * 0.01f, 0.0f, 300.0f);
        // 참고: https://data.ouster.io/downloads/datasheets/datasheet-rev7-v2p5-os2.pdf
        // 참고: https://www.wolframalpha.com/input?i=2.31575756e-04*x%5E2+%2B+-5.70213303e-03*x%5E1+%2B+1.82812720e%2B00%2C+x%3D0+to+x%3D300
        //2.31575756e-04x^2 + -5.70213303e-03x^1 + 1.82812720e+00
        const double stdDevEstimate = (ActiveDescription.NoiseCo2Curve * FMath::Pow(dist, 2))
                                    + (ActiveDescription.NoiseCo1Curve * dist)
                                    + ActiveDescription.NoiseCo0Curve;
        const float noiseStdDev = stdDevEstimate * 0.01f * GLidarWeightNoise;
        const float noiseDistribution = RandomEngine->GetNormalDistribution(0.0f, noiseStdDev) * 0.5f;
        
        // float xRand = FMath::RandRange(-noiseDistribution, noiseDistribution);
        // float yRand = FMath::RandRange(-noiseDistribution, noiseDistribution);
        // float zRand = FMath::RandRange(-noiseDistribution, noiseDistribution);
        // Vector3D noise(xRand, yRand, zRand);
        // Detection.point += noise;

        Vector3D direction = Detection.point;
        float len = direction.Length();
        direction /= len;
        float rangeError = FMath::RandRange(-noiseDistribution, noiseDistribution);
        Detection.point += (direction * rangeError);
        Vector3D noise = Detection.point;
        memcpy(&noise_out, &noise, sizeof(Vector3D));

        //GLog->Logf(TEXT("distance:%f, noiseStdDev:%f"), distance, noiseStdDev);
    }
    // Tick Noise //

    //// Mean Range Error
    //if (ActiveDescription.NoiseMeanRangeError > KINDA_SMALL_NUMBER)
    //{
    //    float meanRangeError = ActiveDescription.NoiseMeanRangeError * 0.5f;
    //    Vector3D direction = Detection.point;
    //    float len = direction.Length();
    //    direction /= len;
    //    float rangeError = FMath::RandRange(-meanRangeError, meanRangeError) * 0.01f;
    //    Detection.point += (direction * rangeError);
    //}
    //// Mean Range Error //
    
    //// Tick Noise 
    //if (ActiveDescription.NoiseStdDev > std::numeric_limits<float>::epsilon()) {
    //    const Vector3D ForwardVector = Detection.point.MakeUnitVector();
    //    const float ratioNoise = (GLidarWeightNoise == 1 ? (FMath::Pow(FMath::Min(distance, range) / range, 0.5f)) : 1.0f); 
    //    const float noiseDistribution = RandomEngine->GetNormalDistribution(0.0f, ActiveDescription.NoiseStdDev)
    //                * ratioNoise;
    //    const Vector3D noise = ForwardVector * noiseDistribution;
    //    Detection.point += noise;
    //    memcpy(&noise_out, &noise, sizeof(Vector3D));
    //}
    //// Tick Noise //

    const float Intensity = Detection.intensity;
    if(Intensity > ActiveDescription.DropOffIntensityLimit)
    {
        return true;
    }
    else
    {
        return RandomEngine->GetUniformFloat() < DropOffAlpha * Intensity + DropOffBeta;
    }
}

void AKetiSensorRayCastLidar::ResetLidarRecordedHits(uint32_t Channels, uint32_t MaxPointsPerChannel)
{
    LidarRecordedHits.resize(Channels);

    for (auto& hits : LidarRecordedHits) {
        hits.clear();
        hits.reserve(MaxPointsPerChannel);
    }
}

void AKetiSensorRayCastLidar::ComputeAndSaveDetectionsKeti(const FTransform& SensorTransform)
{
    const uint32 channels = ActiveDescription.Channels;
    for (auto idxChannel = 0u; idxChannel < channels; ++idxChannel)
        PointsPerChannel[idxChannel] = LidarRecordedHits[idxChannel].size();
    
    KetiLidarData.ResetMemory(PointsPerChannel);

    TAtomic<uint32> CompletedCount(0);
    ParallelFor(channels, [this, &SensorTransform = SensorTransform, &CompletedCount = CompletedCount](int32 idxChannel) {
    //for (auto idxChannel = 0u; idxChannel < channels; ++idxChannel)
    {
        FVector noise;
        int idxCapture = 0, idxRay = 0;
        for (FHitResult& hit : LidarRecordedHits[idxChannel]) {
            if (hit.bBlockingHit == true)
            {
                idxCapture = hit.Item & 0xff;
                idxRay = (hit.Item >> 8) & 0xffffff;
                FKetiSensorCapture& capture = AroundCaptures[idxCapture];
                FCaptureRay& rayHit = capture.CollectRays[idxRay];
                bool bColorPicked = capture.GetColorFromCapture(
                                                        rayHit.Rot,
                                                        rayHit.Point,
                                                        capture.Bitmap,
                                                        rayHit.Color,
                                                        rayHit.UV);
                if (bColorPicked == true)
                {
                    FColor& captureColor = rayHit.Color;
                    hit.Item = rayHit.Hit.Item = captureColor.ToPackedRGBA();
                }
                rayHit.bPicked = true;
            }
    
            FDetection Detection;
            ComputeDetectionSingleKeti(hit, SensorTransform, Detection);
    
            bool bDetected = PostprocessDetectionForKeti(Detection, hit, ActiveDescription.RangeMax, noise);
            if (bDetected == true)
            {
                CountingWriteTest++;
                KetiLidarData.WritePointSync(Detection);
            }
            else
            {
                PointsPerChannel[idxChannel]--;
            }
            AroundCaptures[idxCapture].CollectRays[idxRay].SetDetectResult(
                    Detection.intensity,Detection.point, SensorTransform, bDetected, ActiveDescription.IntensityColors);
        }

        CompletedCount.IncrementExchange();
    }
    });

    while (CompletedCount.Load() < channels)
    {
        // Just wait for completion parallel_for
    }
    
    KetiLidarData.WriteChannelCount(PointsPerChannel);
}

void AKetiSensorRayCastLidar::ComputeAndSaveDetectionsCarla(const FTransform& SensorTransform)
{
    for (auto idxChannel = 0u; idxChannel < ActiveDescription.Channels; ++idxChannel)
        PointsPerChannel[idxChannel] = LidarRecordedHits[idxChannel].size();

    KetiLidarData.ResetMemory(PointsPerChannel);

    int idxCapture = 0, idxRay = 0;
    bool bDetected = false;
    for (auto idxChannel = 0u; idxChannel < ActiveDescription.Channels; ++idxChannel) {
        for (FHitResult& hit : LidarRecordedHits[idxChannel]) {
            FDetection Detection = ComputeDetectionSingleCarla(hit, SensorTransform);
            idxCapture = hit.Item & 0xff;
            idxRay = (hit.Item >> 8) & 0xffffff;
            bDetected = PostprocessDetection(Detection); 
            if (bDetected == true)
            {
                KetiLidarData.WritePointSync(Detection);
            }
            else
            {
                PointsPerChannel[idxChannel]--;
            }   
            AroundCaptures[idxCapture].CollectRays[idxRay].SetDetectResult(Detection.intensity, Detection.point, SensorTransform, bDetected);
        }
    }

    KetiLidarData.WriteChannelCount(PointsPerChannel);
}

void AKetiSensorRayCastLidar::ComputeAndSaveDetections(const FTransform& SensorTransform)
{
    if (GLidarIntensityUseKeti == 1)
    {
        ComputeAndSaveDetectionsKeti(SensorTransform);
    }
    else
    {
        ComputeAndSaveDetectionsCarla(SensorTransform);
    }
}

void AKetiSensorRayCastLidar::TickComputeIntensity()
{
    SCOPE_CYCLE_COUNTER(STAT_LidarTickComputeIntensity);
    if (LidarRecordedHits.size() <= 0) return;
    
    FTransform ActorTransf = GetTransform();
    ComputeAndSaveDetections(ActorTransf);
}

void AKetiSensorRayCastLidar::Clear()
{
    if (CarlaHUD)
        CarlaHUD->SensorHud.RemoveHudSensor(this);
    
    TestCaptureRays.Empty();
    for (int i = 0; i < AroundCaptures.Num(); ++i)
    {
        AroundCaptures[i].CollectRays.Empty();
    }
    TestCaptureSingle.CollectRays.Empty();
    LidarRecordedHits.clear();    
    TraceParams.ClearIgnoredActors();
    FSensorCollection::RemoveByInstance(this);
    if (RTRayRunner)
    {
        if (RTRayRunner->IsValidLowLevelFast() == true)
        RTRayRunner->Destroy();
    }
    
    ACarlaWheeledVehicle* vehicle = Cast<ACarlaWheeledVehicle>(GetOwner());
    if (vehicle->IsValidLowLevelFast())
    {
        UnRegisterCustomEvents(vehicle);
    }
}

void AKetiSensorRayCastLidar::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    Clear();
}

void AKetiSensorRayCastLidar::BeginDestroy()
{
    Super::BeginDestroy();
    Clear();
}

#if WITH_EDITORONLY_DATA
void AKetiSensorRayCastLidar::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    Super::PostEditChangeProperty(e);
    bool isNeedUpdateParameter = e.MemberProperty->GetName().Contains(TEXT("ActiveDescription"));
    if (isNeedUpdateParameter == true)
    {
        ELidarUpdateType updateType = (e.Property->GetName().Contains(TEXT("ChannelCurve"))
                                    || e.Property->GetName().Contains(TEXT("Channels")))
                                    || e.Property->GetName().Contains(TEXT("ResolutionHorizontal")) ? ELidarUpdateType::RecareateLasers :
                                        (e.Property->GetName().Contains(TEXT("ChannelAngles")) ? ELidarUpdateType::UpdateOnlyLasers : ELidarUpdateType::None);
        UpdateDescription(updateType);
    }
}
#endif

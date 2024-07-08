#pragma once

DEFINE_LOG_CATEGORY_STATIC(LogBlessEditor, Log, All);

class CUSTOMEDITOR_API BlessEditorCommon
{
public:
	struct CUSTOMEDITOR_API ActionByCondition
	{
		FString IDAction;
		float TimeStarted;
		float TimeDelayStart;
		float TimeRate;
		float TimeLast;
		float TimeDelayStartBuffer;
		float TimeDelayStartBufferStarted;
		float TimeAbort;
		bool IsLoop;
		
		TFunction<void()> _Action;
		TFunction<bool()> _Condition;

	private:
		bool IsActioned;

	private:
		bool IsTimeToAction(float timeCurr)
		{
			float elapsed = timeCurr - TimeStarted;
			return TimeDelayStart <= 0 || (0 < TimeDelayStart && elapsed > TimeDelayStart);
		}

		bool IsConditioned(float timeCurr)
		{
			if (false == IsTimeToAction(timeCurr))
				return false;

			if (nullptr == _Condition)
				return true;

			return _Condition();
		}

		bool IsRemoveCondition()
		{
			float timeCurr = GetTimeCurrent();
			bool isRemoveCondition = false == IsLoop && true == IsActioned;
			if (false == IsLoop && false == isRemoveCondition && (timeCurr - TimeStarted) > TimeAbort)
			{
				UE_LOG(LogBlessEditor, Warning, TEXT("TimeAbort/ActionByCondition::IsRemoveCondition()/True/%s"), *IDAction);
				isRemoveCondition = true;
			}
			return isRemoveCondition;
		}

	public:
		static void StaticTick(float delta)
		{
			if (ActionByConditions.Num() >= 1)
			{
				for (int i = 0; i < ActionByConditions.Num(); ++i)
				{
					ActionByCondition& actionCurr = ActionByConditions[i];
					actionCurr.TickAction();
				}

				ActionByConditions.RemoveAll([](ActionByCondition& iter) { return iter.IsRemoveCondition(); } );
			}

			if (BlessEditorCommon::ActionByCondition::ActionSingleQueue.Num() >= 1)
			{
				float timeCurrent = GetTimeCurrent();
				if (timeCurrent >= BlessEditorCommon::ActionByCondition::QueueTimeNext)
				{
					TFunction<void()> action = BlessEditorCommon::ActionByCondition::ActionSingleQueue[QueueIndex++];
					action();

					if (QueueIndex >= BlessEditorCommon::ActionByCondition::ActionSingleQueue.Num())
					{
						BlessEditorCommon::ActionByCondition::ActionSingleQueue.Empty();
						QueueIndex = 0;
					}

					BlessEditorCommon::ActionByCondition::QueueTimeNext = timeCurrent + QueueTimeRate;
				}
			}
		}

	public:
		void TickAction()
		{
			if (nullptr == _Action)
				return;

			if (false == IsLoop && true == IsActioned)
				return;

			float timeCurr = GetTimeCurrent();
			if (false == IsConditioned(timeCurr))
				return;

			bool isSkippedByBuffer = false;
			if (false == IsLoop && TimeDelayStartBuffer >= 0)
			{
				if (TimeDelayStartBufferStarted < 0.0f)
				{
					TimeDelayStartBufferStarted = timeCurr;
				}

				isSkippedByBuffer = (timeCurr - TimeDelayStartBufferStarted) <= TimeDelayStartBuffer;
			}

			if (false == isSkippedByBuffer && timeCurr - TimeLast >= TimeRate)
			{
				TimeLast = timeCurr;
				IsActioned = true;
				_Action();
			}
		}

		void Reset() { IsActioned = false; }

		ActionByCondition(FString& idAction, 
						  TFunction<void()> action, 
						  TFunction<bool()> condition, 
						  bool isLoop, 
						  float timeDelayStart, 
						  float timeRate, 
						  float timeDelayStartBuffer)
		{ 
			IDAction = idAction; 
			_Action = action; 
			_Condition = condition; 
			IsLoop = isLoop; 
			TimeDelayStart = timeDelayStart;
			TimeRate = timeRate;
			TimeStarted = GetTimeCurrent();
			TimeLast = 0.0f;
			TimeAbort = 999999.9f;
			IsActioned = false;
			TimeDelayStartBuffer = timeDelayStartBuffer;
			TimeDelayStartBufferStarted = -1.0f;
		}

	private:
		static bool IsInitialized;
		static TArray<ActionByCondition> ActionByConditions;
		static TArray<TFunction<void()>> ActionSingleQueue;
		static float QueueTimeNext;
		static float QueueTimeRate;
		static int QueueIndex;
	
	public:
		static int GetCountActioning() { return ActionByConditions.Num(); }

		static bool IsActioning(const FString& idAction)
		{
			return ActionByConditions.ContainsByPredicate([idAction](const ActionByCondition& iter){ return iter.IDAction.Equals(idAction, ESearchCase::CaseSensitive); });
		}

		static bool IsEmptyActionQueue() { return ActionSingleQueue.Num() <= 0; }

		static bool RemoveAll(FString idContains, FString idExcept = FString())
		{
			if (ActionByConditions.Num() <= 0)
				return false;

			int cntRemoved = ActionByConditions.RemoveAll(
				[&](ActionByCondition& iter) 
				{
					FString idAction = iter.IDAction;
					bool isRemove = ((idExcept.Len() >= 1 && false == idAction.Equals(idExcept, ESearchCase::IgnoreCase))
							|| true)
						&& idAction.Contains(idContains, ESearchCase::IgnoreCase);

					return isRemove;
				});
			return cntRemoved >= 1;
		}

		static FString MakeUniqueID(FString& idRequest)
		{
			int countTry = 0;
			FString idToUse = idRequest;
			while (true)
			{
				bool isContains = ActionByConditions.ContainsByPredicate([&](const ActionByCondition& iter){ return iter.IDAction == idToUse; });
				if (false == isContains)
					break;
				idToUse = FString::Printf(TEXT("%s_%d"), *idRequest, countTry++);
			}
			return idToUse;
		}

		static bool AddActionWithLoop(FString idAction, TFunction<void()> action, TFunction<bool()> condition = nullptr, float timeFirstDelay = -1.0f, float timeRateLooping = 0.1f)
		{
			return AddAction(idAction, true, timeFirstDelay, action, condition, -1.0f, false, timeRateLooping);
		}

		static bool AddAction(FString idAction, TFunction<void()> action, TFunction<bool()> condition = nullptr, float timeDelayStartBuffer = -1.0f)
		{
			return AddAction(idAction, false, -1.0f, action, condition, timeDelayStartBuffer);
		}

		static bool AddAction(	FString idAction, 
								bool isLoop, 
								float timeFirstDelay, 
								TFunction<void()> action, 
								TFunction<bool()> condition = nullptr, 
								float timeDelayStartBuffer = -1.0f, 
								bool isRefreshIfExist = false,
								float timeRateLooping = 0.1f)
		{
			bool isContains = true == ActionByConditions.ContainsByPredicate([&](const ActionByCondition& iter) { return iter.IDAction == idAction; });
			if (true == isContains)
			{
				if (true == isRefreshIfExist)
				{
					ActionByConditions.RemoveAll([&](const ActionByCondition& iter) { return iter.IDAction == idAction; });
				}
				else
				{
					return false;
				}
			}

			ActionByCondition actionByCond = ActionByCondition(idAction, action, condition, isLoop, timeFirstDelay, timeRateLooping, timeDelayStartBuffer);
			ActionByConditions.Add(actionByCond);
			return true;
		}

		static void AddActionQueues(TArray<TFunction<void()>> actionQueue, float timeRate)
		{
			if (ActionSingleQueue.Num() >= 1)
				return;

			ActionSingleQueue = actionQueue;

			QueueTimeRate = timeRate;
			QueueTimeNext = GetTimeCurrent() + timeRate;
			QueueIndex = 0;
		}

		static void StaticInit()
		{
			if (true == IsInitialized)
				return;

			UBLUnrealEdEngine* edEngine = Cast<UBLUnrealEdEngine>(GEditor);
			if (nullptr != edEngine)
			{
				static TFunction<void(float)> actionTick = [=](float delta)
				{
					BlessEditorCommon::TickForEditor(delta);
				};

				edEngine->AddActionOnTick(TEXT("BlessEditorCommon::TickForEditor"), actionTick);
			}

			IsInitialized = true;
		}

		static void ClearAll()
		{
			if (ActionByConditions.Num() <= 0)
				return;

			ActionByConditions.Empty();

			UBLUnrealEdEngine* edEngine = Cast<UBLUnrealEdEngine>(GEditor);
			if (nullptr != edEngine)
			{
				edEngine->RemoveActionOnTick(TEXT("ActionByCondition"));
			}
		}
	};

	struct EquipSlotPair
	{
		FString NamePart;
		EEquipmentPartSlotType SlotType;
		
		static bool IsHasMeshAny(const UBLEquipmentItemInfo* equip)
		{
			if (false == equip->IsValidLowLevelFast())
				return false;

			return equip->EquipInfoSandbox.Num() >= 1;
		}

		bool SetPartIDMatchingType(UBLEquipmentItemInfo* equip) 
		{
			bool isSetPartIDAny = false;
			for (FEquipInfoMapEntity& entity : equip->EquipInfoSandbox)
			{
				UBLEquipmentItemEquipInfo* equipInfo = entity.EquipInfo;
				isSetPartIDAny |= SetPartIDMatching(equipInfo);
			}

			return isSetPartIDAny;
		}

		bool SetPartIDMatching(UBLEquipmentItemEquipInfo* equipInfo)
		{
			if (nullptr == equipInfo || false == equipInfo->IsValidLowLevelFast())
			{
				UE_CLOG(true, LogBlessEditor, Warning, TEXT("ERROR/(nullptr == equipInfo || false == equipInfo->IsValidLowLevelFast())/equip:%s"), *(nullptr == equipInfo ? TEXT("null") : equipInfo->GetName()));
				return false;
			}
				
			if (false == equipInfo->SkelMesh.GetAssetName().Contains(NamePart))
			{
				return false;
			}

			equipInfo->PartID = SlotType;
			return true;
		}

		EquipSlotPair(FString namePart, EEquipmentPartSlotType slotType)
		{ 
			NamePart = namePart; SlotType = slotType;
		}
	};

	static TArray<EquipSlotPair> EquipSlots;
	static uint32 TimeCycleStart;

	static void StaticInit()
	{
		TimeCycleStart = FPlatformTime::Cycles();

		ActionByCondition::StaticInit();

		if (0 >= EquipSlots.Num())
		{
			EquipSlots.Add(EquipSlotPair(TEXT("_Upper"), EEquipmentPartSlotType::Upper));
			EquipSlots.Add(EquipSlotPair(TEXT("_Lower"), EEquipmentPartSlotType::Lower));
			EquipSlots.Add(EquipSlotPair(TEXT("_face" ), EEquipmentPartSlotType::HeadFace));
			EquipSlots.Add(EquipSlotPair(TEXT("_hair" ), EEquipmentPartSlotType::Hair));
			EquipSlots.Add(EquipSlotPair(TEXT("_boots"), EEquipmentPartSlotType::Boots));
			EquipSlots.Add(EquipSlotPair(TEXT("_glove"), EEquipmentPartSlotType::Glove));
			EquipSlots.Add(EquipSlotPair(TEXT("_Helmet"), EEquipmentPartSlotType::Head));
			EquipSlots.Add(EquipSlotPair(TEXT("_Shoulder"), EEquipmentPartSlotType::Shoulder));
			EquipSlots.Add(EquipSlotPair(TEXT("_Ear"), EEquipmentPartSlotType::Ear));
			EquipSlots.Add(EquipSlotPair(TEXT("_Tail"), EEquipmentPartSlotType::Tail));
			EquipSlots.Add(EquipSlotPair(TEXT("_BeardUpper"), EEquipmentPartSlotType::BeardUpper));
			EquipSlots.Add(EquipSlotPair(TEXT("_BeardLower"), EEquipmentPartSlotType::BeardLower));
			EquipSlots.Add(EquipSlotPair(TEXT("_Belt"), EEquipmentPartSlotType::Belt));
		}
	}

	static void TickForEditor(float delta)
	{
		ActionByCondition::StaticTick(delta);
	};

	static uint32 GetTimeCycleCurrent()
	{
		return FPlatformTime::Cycles() - TimeCycleStart;
	}

	static float GetTimeCurrent()
	{
		return FPlatformTime::ToMilliseconds(GetTimeCycleCurrent()) * 0.001f;
	}

	static uint32 GetUniqueIDNew()
	{
		return GetTimeCycleCurrent();
	}

public:
	static void AddCheckout(UObject* objectToSave, bool isSaveWith = false)
	{
		if (nullptr == objectToSave || false == objectToSave->IsValidLowLevel())
			return; 

		UPackage* Package = objectToSave->GetOutermost();
		if (Package)
		{
			FString PackageFilename;
			if (FPackageName::TryConvertLongPackageNameToFilename(Package->GetName(), PackageFilename, FPackageName::GetAssetPackageExtension()))
			{
				// checkout or markforadd
				AddCheckoutPackage(Package);

				if (true == isSaveWith)
				{
					bool isSaved = SavePackageHelper(Package, PackageFilename);
					if (false == isSaved)
					{
						UE_LOG(LogBlessEditor, Error, TEXT("(false == isSaved)/AddCheckout/%s"), *PackageFilename);
					}
				}
			}
		}
	}

	static void CloseEditorsAll(UObject* asset)
	{
		FAssetEditorManager& AssetEditorManager = FAssetEditorManager::Get();
		AssetEditorManager.CloseAllEditorsForAsset(asset);
		GEditor->ForceGarbageCollection();
	}

private:
	static void AddCheckoutPackage(const UPackage* InPackage)
	{
		ISourceControlProvider& SourceControlProvider = ISourceControlModule::Get().GetProvider();
		if (!SourceControlProvider.IsAvailable())
		{
			SourceControlProvider.Login();
		}
		if (SourceControlProvider.IsAvailable())
		{
			SourceControlProvider.Execute(ISourceControlOperation::Create<FUpdateStatus>(), InPackage);
			FSourceControlStatePtr SourceControlState = SourceControlProvider.GetState(InPackage, EStateCacheUsage::Use);
			if (SourceControlState.IsValid())
			{
				if (SourceControlState->CanAdd())
				{
					SourceControlProvider.Execute(ISourceControlOperation::Create<FMarkForAdd>(), InPackage);
				}
				else
				{
					SourceControlProvider.Execute(ISourceControlOperation::Create<FCheckOut>(), InPackage);
				}
			}
		}
	}

public:
	static FString GetPathAbsFromRelative(const FString& pathRelative)
	{
		static FString pathSubstract = TEXT("/Game/");
		FString path = pathRelative;
		int idx = path.Find(pathSubstract, ESearchCase::IgnoreCase);
		if (idx == 0)
		{
			path = path.Right(path.Len() - pathSubstract.Len());
		}

		path = FPaths::ProjectContentDir() + path;
		return path;
	}

	static FString GetPathRelativeFromAbs(const FString& pathAbs, bool isIncludeExt = true, bool isRelativeExtWith = false)
	{
		static FString pathFind = TEXT("Bless/Content");
		static FString pathAdd = TEXT("/Game");
		FString path = pathAbs;
		path = path.Replace(TEXT("\\"), TEXT("/"));
		int idx = path.Find(pathFind, ESearchCase::IgnoreCase);
		if (INDEX_NONE == idx)
		{
			return FString();
		}

		path = pathAdd + path.Right(path.Len() - (idx + pathFind.Len()));
		
		if (true == isIncludeExt)
		{
			if (true == isRelativeExtWith)
			{
				path = GetFileNameExceptExt(path) + TEXT(".") + GetFileNamePure(path);
			}
		}
		else
		{
			path = GetFileNameExceptExt(path);
		}

		return path;
	}

	static FString GetPathAssetNotified(const FString& pathRelative)
	{
		FString path = pathRelative;
		if (path.IsEmpty() 
			|| path.Len() <= 0 
			|| path.Replace(TEXT(" "), TEXT("")).Len() <= 0
			|| false == path.Contains(TEXT("/")))
			return path;

		const static FString dot = TEXT(".");
		int idx = pathRelative.Find(dot, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		
		if (idx == INDEX_NONE)
		{
			path = GetFileNameExceptExt(path) + TEXT(".") + GetFileNamePure(path);
		}
		else
		{
			path = pathRelative;
		}

		return path;
	}

	static FString GetFileNamePure(FString fileName)
	{
		return GetFileNameExceptPath(fileName, false);
	}

	static FString GetFileNameExceptExt(FString fileName)
	{
		const static FString dot = TEXT(".");
		int idx = fileName.Find(dot, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (idx == INDEX_NONE)
		{
			return fileName;
		}
		else
		{
			return fileName.Left(idx);
		}
	}

	static FString GetFileNameExceptPath(FString path, bool isExtWith = false)
	{
		static TArray<FString> slashes;
		if (slashes.Num() <= 0)
		{
			slashes.Add(TEXT("/")); 
			slashes.Add(TEXT("\\"));
		}
		
		int idx = INDEX_NONE;
		for (int i = 0; i < slashes.Num(); ++i)
		{
			idx = path.Find(slashes[i], ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			if (idx != INDEX_NONE)
				break;
		}
		
		FString resultFileName;
		if (idx == INDEX_NONE)
		{
			resultFileName = true == isExtWith ? path : GetFileNameExceptExt(path);
		}
		else
		{
			FString pathSubstring = path.Right(path.Len() - idx - 1);
			resultFileName = true == isExtWith ? pathSubstring : GetFileNameExceptExt(pathSubstring);
		}

		return resultFileName;
	}

	static FString GetPathExceptFileName(FString path)
	{
		static TArray<FString> slashes;
		if (slashes.Num() <= 0)
		{
			slashes.Add(TEXT("/")); 
			slashes.Add(TEXT("\\"));
		}
		
		int idx = INDEX_NONE;
		for (int i = 0; i < slashes.Num(); ++i)
		{
			idx = path.Find(slashes[i], ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			if (idx != INDEX_NONE)
				break;
		}
		
		if (idx == INDEX_NONE)
		{
			return path;
		}
		else
		{
			return path.Left(idx);
		}
	}

	template <class FunctorType>
	class PlatformFileFunctor : public IPlatformFile::FDirectoryVisitor	//GenericPlatformFile.h
	{
	public:
	
		virtual bool Visit(const TCHAR* iterPath, bool bIsDirectory) override
		{
			return Functor(iterPath, bIsDirectory);
		}

		PlatformFileFunctor(FunctorType&& FunctorInstance)
			: Functor(MoveTemp(FunctorInstance))
		{
		}

	private:
		FunctorType Functor;
	};

private:
	template <class Functor>
	static PlatformFileFunctor<Functor> MakeDirectoryVisitor(Functor&& FunctorInstance)
	{
		return PlatformFileFunctor<Functor>(MoveTemp(FunctorInstance));
	}

public:
	//@ return Key = File Name / Value = Path.
	static bool GetFiles(const FString& pathDirAbsolute, 
						 bool isRecursive,
						 TArray<TPair<FString, FString>>& fileNames_out, 
						 bool isExceptExt = true,
						 bool isEngineExtWith = false,
						 const FString& filterExtension = "",
						 TArray<FString>* exceptFolderNames = nullptr)
	{
		const FString FileExt = filterExtension.Replace(TEXT("."),TEXT("")).ToLower();
	
		const static FString dot = TEXT(".");
		FString fileNameStr;
		FString pathApp(FPaths::GameContentDir());
		auto FilenamesVisitor = MakeDirectoryVisitor(
			[&](const TCHAR* iterPath, bool bIsDirectory) 
			{
				if (true == bIsDirectory)
					return true;

				if (nullptr != exceptFolderNames && 1 <= exceptFolderNames->Num())
				{
					FString pathCheck(iterPath);
					if (true == exceptFolderNames->ContainsByPredicate([&](FString& iter)
						{ return pathCheck.Contains(iter); }))
						return true;
				}

				TPair<FString, FString> pathPair;
				FString path(iterPath);
				if (path.Len() > pathApp.Len())
				{
					path = path.Right(path.Len() - pathApp.Len());
				}

				fileNameStr = FPaths::GetCleanFilename(iterPath);
				FString fileNamePure = GetFileNameExceptExt(fileNameStr);

				path = TEXT("/Game/") + path;
				int idx = path.Find(dot, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				bool isExistExt = idx != INDEX_NONE;
				if (true == isEngineExtWith)
				{
					if (false == isExistExt)
					{
						path = path + dot + fileNamePure;
					}
				}
				else
				{
					if (true == isExistExt)
					{
						path = path.Left(idx);
					}
				}

				pathPair.Value = path;

				if(FileExt != "")
				{				
					if(FPaths::GetExtension(fileNameStr).ToLower() == FileExt) 
					{
						pathPair.Key = fileNameStr;
						fileNames_out.AddUnique(pathPair);
					}
				}
				else
				{
					pathPair.Key = fileNameStr;
					fileNames_out.AddUnique(pathPair);
				}

				return true;
			}
		);

		bool isDone = false;
		if(isRecursive) 
		{
			isDone = FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*pathDirAbsolute, FilenamesVisitor);
		}
		else 
		{
			isDone = FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*pathDirAbsolute, FilenamesVisitor);
		}

		if (true == isDone && true == isExceptExt)
		{
			for (TPair<FString, FString>& iter : fileNames_out)
			{
				FString fileName = GetFileNameExceptExt(iter.Key);
				if (true == isEngineExtWith)
				{
					FString fileNamePure = GetFileNameExceptPath(fileName, false);
					fileName = fileName + "." + fileNamePure;
				}

				iter.Key = fileName.Len() <= 0 ? iter.Key : fileName;
			}
		}

		return isDone;
	}

	static bool GetDirectories(const FString& pathDirAbsolute, 
							   TArray<FString>& DirsOut, 
							   bool isRecursive = false, 
							   const FString& ContainsStr="", 
							   TArray<FString>* exceptDirNames = nullptr)
	{
		FString Str;
		auto FilenamesVisitor = MakeDirectoryVisitor(
			[&](const TCHAR* iterPath, bool bIsDirectory) 
			{
				if (false == bIsDirectory)
					return true;

				if (nullptr != exceptDirNames && exceptDirNames->Num() >= 1)
				{
					FString pathCurr(iterPath);
					if (exceptDirNames->ContainsByPredicate([&](FString& iter) { return pathCurr.Contains(iter); }))
					{
						return true;
					}
				}

				if(ContainsStr != "")
				{
					Str = FPaths::GetCleanFilename(iterPath);
					
					if(Str.Contains(ContainsStr))
					{
						if(isRecursive) DirsOut.Push(iterPath);
						else DirsOut.Push(Str);
					}
				}
				else
				{
					Str = FPaths::GetCleanFilename(iterPath);
					
					if(isRecursive) DirsOut.Push(iterPath);
					else DirsOut.Push(Str);
				}

				return true;
			}
		);

		if(isRecursive) 
		{
			return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*pathDirAbsolute, FilenamesVisitor);
		}
		else 
		{
			return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*pathDirAbsolute, FilenamesVisitor);
		}
	}

	static bool LoadAssetsAll(const FString& path, TArray<FAssetData>& loads_out)
	{
		TArray<FString> paths;
		paths.Add(path);
		return LoadAssetsAll(paths, true, loads_out);
	}

	static bool LoadAssetsAll(TArray<FString> paths, bool isRecursive, TArray<FAssetData>& loads_out)
	{
		UObjectLibrary* library = UObjectLibrary::CreateLibrary(UObject::StaticClass(), false, true);
		if (library == nullptr)
		{
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(TEXT("(library == nullptr)")));
			return false;
		}

		library->bRecursivePaths = isRecursive;
		
		int numLoad = library->LoadAssetDataFromPaths(paths);
		if (numLoad <= 0)
			return false;

		library->LoadAssetsFromAssetData();
		library->GetAssetDataList(loads_out);

		return true;
	}
public:
	template<typename ObjectType>
	static FString GetNamePathPackageString(TAssetPtr<ObjectType>& obj, bool isExtWith = false)
	{
		return true == isExtWith ? obj.ToString() : GetFileNameExceptExt(obj.ToString());
	}

	static FString GetNamePackageString(const UObject* obj, bool isExtWith = false)
	{
		if (nullptr == obj)
			return FString();

		UPackage* package = obj->GetOutermost();
		check(package);
		return GetFileNameExceptPath(package->GetName(), isExtWith);
	}

	static FString GetNamePathPackageString(const UObject* obj, bool isExtWith = false)
	{
		if (nullptr == obj)
			return FString();

		UPackage* package = obj->GetOutermost();
		check(package);

		return true == isExtWith ? package->GetPathName() : package->GetName();
	}

	static bool IsExistAssetFile(UObject* obj)
	{
		if (nullptr == obj)
			return false;

		FString pathAsset = GetNamePathPackageString(obj) + TEXT(".uasset");
		pathAsset = GetPathAbsFromRelative(pathAsset);
		return FPaths::FileExists(pathAsset);
	}

	static bool IsExistAssetFile(FString pathRelative, FString fileExtension = TEXT("uasset"))
	{
		FString pathAsset = GetPathAbsFromRelative(pathRelative) + TEXT(".") + fileExtension;
		return FPaths::FileExists(pathAsset);
	}

	static FName GetNamePackage(const UObject* obj)
	{
		FString nameString = GetNamePathPackageString(obj, true);
		FString nameAsset = GetFileNameExceptPath(nameString, true);
		if (false == nameAsset.Contains(TEXT(".")))
		{
			nameString = nameString + "." + nameAsset;
		}

		FName namePackage = FName(*(nameString));
		return namePackage;
	}

public:
	//@ ex) path="DataPackage/BodyInfoSetting"
	static bool ExecAssetIteration(FString path, TFunction<void(FAssetData&)> predicate, bool isRecursivePath = false)
	{
		TArray<FAssetData> loads;
		FString pathPC(TEXT("/Game/") + path);
		TArray<FString> paths;
		paths.Add(pathPC);

		bool isLoad = LoadAssetsAll(paths, isRecursivePath, loads);
		if (false == isLoad)
		{
			UE_LOG(LogBlessEditor, Error, TEXT("(false == isLoad)/ExecAssetIteration"));
		}
		else
		{
			for (int i = 0; i < loads.Num(); ++i)
			{
				FAssetData& assetLoad = loads[i];
				if (false == assetLoad.IsValid())
				{
					continue;
				}

				predicate(assetLoad);
			}
		}

		return true;
	}

public:
	static void ExecPerformMeasureTest(int count)
	{
		TArray<UObject*> objects;
		{
			ScopeSimpleProfile(PerformanceTestStringObjects);
			GetObjectsAll(objects, nullptr);
		}

		if (0 >= count)
		{
			return;
		}

		static TArray<FName> testNames;
		{
			ScopeSimpleProfile(PerformanceTestName);
			for (int i = 0; i < count; ++i)
			{
				testNames.Add(FName(*FString::Printf(TEXT("TestName_%d"), i)));
			}
		}

		static TArray<FString> testStrings; testStrings.Empty();
		{
			ScopeSimpleProfile(PerformanceTestString);
			for (int i = 0; i < count; ++i)
			{
				FName& name = testNames[i];
				testStrings.Add(name.ToString());
			}
		}

		static TArray<FString> testString2s; testString2s.Empty();
		{
			ScopeSimpleProfile(PerformanceTestString2);
			for (int i = 0; i < count; ++i)
			{
				testString2s.Add(FString::Printf(TEXT("Test%s_Test02%d_Test03%f"), *testStrings[i], i, float(i)));
			}
		}

		{
			testNames.Empty();
			ScopeSimpleProfile(PerformanceTestObjectGetFName);
			int countIteration = FMath::Min(objects.Num(), count);
			for (int i = 0; i < countIteration; ++i)
			{
				int idx = i;
				if (idx >= countIteration)
				{
					while(idx < countIteration)
						idx = idx - countIteration;
				}
					
				UObject* obj = objects[idx];
				testNames.Add(obj->GetFName());
			}
		}

		{
			testStrings.Empty();
			ScopeSimpleProfile(PerformanceTestObjectGetName);
			int countIteration = FMath::Min(objects.Num(), count);
			for (int i = 0; i < count; ++i)
			{
				int idx = i;
				if (idx >= countIteration)
				{
					while(idx >= countIteration)
						idx = idx - countIteration;

					if (idx <= -1)
						idx = 0;
				}
					
				UObject* obj = objects[idx];
				testStrings.Add(obj->GetName());
			}
		}

		static std::vector<FString> testStringShuffles;
		testStringShuffles.clear();
		for (int idx = 0; idx < testStrings.Num(); ++idx)
		{
			testStringShuffles.push_back(testStrings[idx]);
		}

		if (testStringShuffles.size() >= 2)
		{
			{
				ScopeSimpleProfile(PerformanceTestRandom_shuffle);
				std::random_shuffle(testStringShuffles.begin(), testStringShuffles.end());
			}
			
			{
				ScopeSimpleProfile(PerformanceTestTArrayFind);
				TArray<int> testFounds;
				for (int i = 0; i < testStringShuffles.size(); ++i)
				{
					int idxFound = testStrings.Find(testStringShuffles[i]);
					testFounds.Add(idxFound);
				}
			}
			
			//@ Extremely slow. below case x 100 times.
			//{
			//	ScopeSimpleProfile(PerformanceTestRandom_shuffle);
			//	std::random_shuffle(testStringShuffles.begin(), testStringShuffles.end());
			//}
			//
			//const static FString findKeyString = TEXT("World");
			//{
			//	ScopeSimpleProfile(PerformanceTestTArrayFindPredicateString);
			//	TArray<FString*> testFounds;
			//	for (int i = 0; i < testStringShuffles.size(); ++i)
			//	{
			//		//FString* found = testStrings.FindByPredicate([=](FString& iter){ return iter.Contains(testStringShuffles[i]); });
			//		FString* found = testStrings.FindByPredicate([=](FString& iter){ return iter.Contains(findKeyString, ESearchCase::IgnoreCase); });
			//		testFounds.Add(found);
			//	}
			//}
			//
			//{
			//	ScopeSimpleProfile(PerformanceTestRandom_shuffle);
			//	std::random_shuffle(testStringShuffles.begin(), testStringShuffles.end());
			//}
			//
			//{
			//	ScopeSimpleProfile(PerformanceTestTArrayFindPredicateString3);
			//	TArray<FString*> testFounds;
			//	for (int i = 0; i < testStringShuffles.size(); ++i)
			//	{
			//		//FString* found = testStrings.FindByPredicate([&](FString& iter){ return iter.Equals(testStringShuffles[i]); });
			//		FString* found = testStrings.FindByPredicate([&](FString& iter){ return iter.Equals(findKeyString, ESearchCase::IgnoreCase); });
			//		testFounds.Add(found);
			//	}
			//}

			{
				ScopeSimpleProfile(PerformanceTestTArrayFindString4);
				TArray<FString*> testFounds;
				for (int i = 0; i < testStringShuffles.size(); ++i)
				{
					FString* found = testStrings.FindByKey(testStringShuffles[i]);
					testFounds.Add(found);
				}
			}

			{
				ScopeSimpleProfile(PerformanceTestTArrayFindString5);
				TArray<FString> testFounds;
				for (int i = 0; i < testStringShuffles.size(); ++i)
				{
					int idxFound = 0;
					testStrings.Find(testStringShuffles[i], idxFound);
					if (INDEX_NONE != idxFound)
						testFounds.Add(testStrings[idxFound]);
				}
			}

			{
				ScopeSimpleProfile(PerformanceTestTArrayFindString6);
				TArray<FString> testFounds;
				for (int i = 0; i < testStringShuffles.size(); ++i)
				{
					bool contains = testStrings[i].Contains(testStringShuffles[i]);
					if (true == contains)
						testFounds.Add(testStrings[i]);
				}
			}

			{
				ScopeSimpleProfile(PerformanceTestTArrayFindString7);
				TArray<FString> testFounds;
				for (int i = 0; i < testStringShuffles.size(); ++i)
				{
					bool equals = testStrings[i].Equals(testStringShuffles[i]);
					if (true == equals)
						testFounds.Add(testStrings[i]);
				}
			}
		}

		TArray<int> testInts;
		static std::vector<int> testIntShuffles;
		for (int idx = 0; idx < count; ++idx)
		{
			int number = FMath::RandRange(0, 1 << 30);
			testInts.Add(number);
			testIntShuffles.push_back(number);
		}

		if (testStringShuffles.size() >= 2)
		{
			{
				std::random_shuffle(testIntShuffles.begin(), testIntShuffles.end());
			}
			
			{
				ScopeSimpleProfile(PerformanceTestTArrayFindPredicateInt);
				TArray<int> testFounds;
				for (int i = 0; i < testIntShuffles.size(); ++i)
				{
					int* idxFound = testInts.FindByPredicate([=](int& iter){ return iter == testIntShuffles[i]; });
					if (idxFound)
						testFounds.Add(*idxFound);
				}
			}
		}

		static TArray<FVector> testSampleVectors;

		{
			testSampleVectors.Empty();
			ScopeSimpleProfile(PerformanceTestVectorDataAdd);
			for (int i = 0; i < count; ++i)
			{
				testSampleVectors.Add(FVector(FMath::Rand(), FMath::Rand(), FMath::Rand()));
			}
		}

		{
			static TArray<FVector> testVectors; testVectors.Empty();
			ScopeSimpleProfile(PerformanceTestVectorNormalize);
			for (int i = 0; i < count; ++i)
			{
				FVector sampleVector = testSampleVectors[i];
				sampleVector.Normalize();
				testVectors.Add(sampleVector);
			}
		}

		{
			static TArray<FVector> testVectorsUnSafe; testVectorsUnSafe.Empty();
			ScopeSimpleProfile(PerformanceTestVectorNormalizeUnSafe);
			for (int i = 0; i < count; ++i)
			{
				FVector sampleVector = testSampleVectors[i];
				testVectorsUnSafe.Add(sampleVector.GetUnsafeNormal());
			}
		}

		{
			static TArray<FVector> testVectorsSafe; testVectorsSafe.Empty();
			ScopeSimpleProfile(PerformanceTestVectorNormalizeSafe);
			for (int i = 0; i < count; ++i)
			{
				FVector sampleVector = testSampleVectors[i];
				testVectorsSafe.Add(sampleVector.GetSafeNormal());
			}
		}

		static TArray<float> testSampleScalars;
		{
			testSampleScalars.Empty();
			ScopeSimpleProfile(PerformanceTestScalarAdd);
			for (int i = 0; i < count; ++i)
			{
				testSampleScalars.Add(FMath::Rand());
			}
		}

		{
			static TArray<float> testScalarsAcos; testScalarsAcos.Empty();
			ScopeSimpleProfile(PerformanceTestScalarACos);
			for (int i = 0; i < count; ++i)
			{
				testScalarsAcos.Add(FMath::Acos(testSampleScalars[i]));
			}
		}
	}
};

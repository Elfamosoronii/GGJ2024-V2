/*******************************************************************************
The content of this file includes portions of the proprietary AUDIOKINETIC Wwise
Technology released in source code form as part of the game integration package.
The content of this file may not be used without valid licenses to the
AUDIOKINETIC Wwise Technology.
Note that the use of the game engine is subject to the Unreal(R) Engine End User
License Agreement at https://www.unrealengine.com/en-US/eula/unreal
 
License Usage
 
Licensees holding valid licenses to the AUDIOKINETIC Wwise Technology may use
this file in accordance with the end user license agreement provided with the
software or, alternatively, in accordance with the terms contained
in a written agreement between you and Audiokinetic Inc.
Copyright (c) 2023 Audiokinetic Inc.
*******************************************************************************/

#include "Wwise/WwiseMediaFileState.h"
#include "Wwise/WwiseMediaManager.h"
#include "Wwise/WwiseStreamingManagerHooks.h"
#include "Wwise/API/WwiseSoundEngineAPI.h"
#include "Wwise/Stats/AsyncStats.h"
#include "AkUnrealHelper.h"
#include "Async/MappedFileHandle.h"

#include <inttypes.h>

FWwiseMediaFileState::FWwiseMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath) :
	FWwiseMediaCookedData(InCookedData),
	RootPath(InRootPath)
{
	INC_DWORD_STAT(STAT_WwiseFileHandlerKnownMedia);
}

FWwiseMediaFileState::~FWwiseMediaFileState()
{
	DEC_DWORD_STAT(STAT_WwiseFileHandlerKnownMedia);
}

FWwiseInMemoryMediaFileState::FWwiseInMemoryMediaFileState(const FWwiseMediaCookedData& InCookedData, const FString& InRootPath) :
	FWwiseMediaFileState(InCookedData, InRootPath)
{
	pMediaMemory = nullptr;
	sourceID = MediaId;
	uMediaSize = 0;
}

void FWwiseInMemoryMediaFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryMediaFileState::OpenFile"));
	if (UNLIKELY(uMediaSize || pMediaMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryMediaFileState::OpenFile %" PRIu32 " (%s): Seems to be already opened."), MediaId, *DebugName.ToString());
		return OpenFileFailed(MoveTemp(InCallback));
	}

	const auto FullPathName = RootPath / MediaPathName.ToString();

	int64 FileSize = 0;
	if (LIKELY(GetFileToPtr(const_cast<const uint8*&>(pMediaMemory), FileSize, FullPathName, bDeviceMemory, MemoryAlignment, true)))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseInMemoryMediaFileState::OpenFile %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
		uMediaSize = FileSize;
		return OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryMediaFileState::OpenFile %" PRIu32 " (%s): Failed to open In-Memory Media (%s)."), MediaId, *DebugName.ToString(), *FullPathName);
		pMediaMemory = nullptr;
		FileSize = 0;
		return OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryMediaFileState::LoadInSoundEngine"));
	if (UNLIKELY(!uMediaSize || !pMediaMemory))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryMediaFileState::LoadInSoundEngine %" PRIu32 " (%s): No data, but supposed to be loaded."), MediaId, *DebugName.ToString());
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseInMemoryMediaFileState::LoadInSoundEngine %" PRIu32 " (%s): Failed loading media without a SoundEngine."), MediaId, *DebugName.ToString());
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}

	const auto SetMediaResult = SoundEngine->SetMedia(this, 1);
	if (LIKELY(SetMediaResult == AK_Success))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseInMemoryMediaFileState::LoadInSoundEngine %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
		INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		return LoadInSoundEngineSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryMediaFileState::LoadInSoundEngine %" PRIu32 " (%s): Failed to load Media: %d (%s)."), MediaId, *DebugName.ToString(), SetMediaResult, AkUnrealHelper::GetResultString(SetMediaResult));
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryMediaFileState::UnloadFromSoundEngine"));
	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseInMemoryMediaFileState::UnloadFromSoundEngine %" PRIu32 " (%s): Failed unloading media without a SoundEngine."), MediaId, *DebugName.ToString());
		return CloseFileDone(MoveTemp(InCallback));
	}

	const auto Result = SoundEngine->TryUnsetMedia(this, 1, nullptr);
	if (UNLIKELY(Result == AK_ResourceInUse))
	{
		UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseInMemoryMediaFileState::UnloadFromSoundEngine %" PRIu32 " (%s): Deferred."), MediaId, *DebugName.ToString());
		return UnloadFromSoundEngineDefer(MoveTemp(InCallback));
	}
	else
	{
		UE_CLOG(UNLIKELY(Result != AK_Success), LogWwiseFileHandler, Error, TEXT("FWwiseInMemoryMediaFileState::UnloadFromSoundEngine %" PRIu32 " (%s): TryUnsetMedia failed: %d (%s)"), MediaId, *DebugName.ToString(), Result, AkUnrealHelper::GetResultString(Result));
		UE_CLOG(LIKELY(Result == AK_Success), LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseInMemoryMediaFileState::UnloadFromSoundEngine %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
		DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		return UnloadFromSoundEngineDone(MoveTemp(InCallback));
	}
}

void FWwiseInMemoryMediaFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseInMemoryMediaFileState::CloseFile"));
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseInMemoryMediaFileState::CloseFile %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
	DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true);
	pMediaMemory = nullptr;
	uMediaSize = 0;
	CloseFileDone(MoveTemp(InCallback));
}

FWwiseStreamedMediaFileState::FWwiseStreamedMediaFileState(const FWwiseMediaCookedData& InCookedData,
	const FString& InRootPath, uint32 InStreamingGranularity) :
	FWwiseMediaFileState(InCookedData, InRootPath),
	StreamingGranularity(InStreamingGranularity),
	StreamedFile(nullptr)
{
	sourceID = InCookedData.MediaId;
	pMediaMemory = nullptr;
	uMediaSize = 0;
}

void FWwiseStreamedMediaFileState::CloseStreaming()
{
	auto* MediaManager = IWwiseMediaManager::Get();
	if (UNLIKELY(!MediaManager))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseStreamedMediaFileState::CloseStreaming %" PRIu32 " (%s): Closing without a MediaManager."), MediaId, *DebugName.ToString());
		return;
	}
	MediaManager->GetStreamingHooks().CloseStreaming(MediaId, *this);
}

void FWwiseStreamedMediaFileState::OpenFile(FOpenFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedMediaFileState::OpenFile"));
	if (UNLIKELY(iFileSize != 0 || StreamedFile))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::OpenFile %" PRIu32 " (%s): Stream seems to be already opened."), MediaId, *DebugName.ToString());
		return OpenFileFailed(MoveTemp(InCallback));
	}

	if (PrefetchSize == 0)
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedMediaFileState::OpenFile %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
		return OpenFileSucceeded(MoveTemp(InCallback));
	}

	// Process PrefetchSize and send as SetMedia
	const auto FullPathName = RootPath / MediaPathName.ToString();

	int64 FileSize = 0;
	if (UNLIKELY(!GetFileToPtr(const_cast<const uint8*&>(pMediaMemory), FileSize, FullPathName, bDeviceMemory, MemoryAlignment, true, PrefetchSize)))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::OpenFile %" PRIu32 " (%s): Failed to Read prefetch media (%s)."), MediaId, *DebugName.ToString(), *FullPathName);
		pMediaMemory = nullptr;
		return OpenFileFailed(MoveTemp(InCallback));
	}
	uMediaSize = FileSize;

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseStreamedMediaFileState::OpenFile %" PRIu32 " (%s): Failed prefetch media without a SoundEngine."), MediaId, *DebugName.ToString());
		DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true);
		pMediaMemory = nullptr;
		uMediaSize = 0;
		return OpenFileFailed(MoveTemp(InCallback));
	}

	const auto SetMediaResult = SoundEngine->SetMedia(this, 1);
	if (LIKELY(SetMediaResult == AK_Success))
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedMediaFileState::OpenFile %" PRIu32 " (%s): Prefetched %" PRIu32 " bytes."), MediaId, *DebugName.ToString(), uMediaSize);
		INC_MEMORY_STAT_BY(STAT_WwiseFileHandlerPrefetchMemoryAllocated, uMediaSize);
		INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		return OpenFileSucceeded(MoveTemp(InCallback));
	}
	else
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::OpenFile %" PRIu32 " (%s): Failed to prefetch media: %d (%s)."), MediaId, *DebugName.ToString(), SetMediaResult, AkUnrealHelper::GetResultString(SetMediaResult));
		DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true);
		pMediaMemory = nullptr;
		uMediaSize = 0;
		return OpenFileFailed(MoveTemp(InCallback));
	}
}

void FWwiseStreamedMediaFileState::LoadInSoundEngine(FLoadInSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedMediaFileState::LoadInSoundEngine"));
	if (UNLIKELY(iFileSize != 0 || StreamedFile))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::LoadInSoundEngine %" PRIu32 " (%s): Stream seems to be already loaded."), MediaId, *DebugName.ToString());
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}

	FWwiseFileCache* FileCache = FWwiseFileCache::Get();
	if (UNLIKELY(!FileCache))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::LoadInSoundEngine %" PRIu32 " (%s): WwiseFileCache not available."), MediaId, *DebugName.ToString());
		return LoadInSoundEngineFailed(MoveTemp(InCallback));
	}

	const auto FullPathName = RootPath / MediaPathName.ToString();

	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamedMediaFileState::LoadInSoundEngine %" PRIu32 " (%s): Opening file"), MediaId, *DebugName.ToString());
	FileCache->CreateFileCacheHandle(StreamedFile, FullPathName, [this, Callback = MoveTemp(InCallback)](bool bResult) mutable
	{
		if (UNLIKELY(!bResult))
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::LoadInSoundEngine %" PRIu32 ": Failed to load Streaming Media (%s)."), MediaId, *DebugName.ToString());
			FFunctionGraphTask::CreateAndDispatchWhenReady([StreamedFile=StreamedFile]
			{
				delete StreamedFile;
			});
			StreamedFile = nullptr;
			return LoadInSoundEngineFailed(MoveTemp(Callback));
		}

		iFileSize = StreamedFile->GetFileSize();
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedMediaFileState::LoadInSoundEngine %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
		INC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
		return LoadInSoundEngineSucceeded(MoveTemp(Callback));
	});
}

void FWwiseStreamedMediaFileState::UnloadFromSoundEngine(FUnloadFromSoundEngineCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedMediaFileState::UnloadFromSoundEngine"));
	UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedMediaFileState::UnloadFromSoundEngine %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());

	const auto* StreamedFileToDelete = StreamedFile;
	StreamedFile = nullptr;
	iFileSize = 0;

	delete StreamedFileToDelete;

	DEC_DWORD_STAT(STAT_WwiseFileHandlerLoadedMedia);
	UnloadFromSoundEngineDone(MoveTemp(InCallback));
}

void FWwiseStreamedMediaFileState::CloseFile(FCloseFileCallback&& InCallback)
{
	SCOPED_WWISEFILEHANDLER_EVENT_3(TEXT("FWwiseStreamedMediaFileState::CloseFile"));
	if (pMediaMemory == nullptr)
	{
		UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedMediaFileState::CloseFile %" PRIu32 " (%s)"), MediaId, *DebugName.ToString());
		return CloseFileDone(MoveTemp(InCallback));
	}

	auto* SoundEngine = IWwiseSoundEngineAPI::Get();
	if (UNLIKELY(!SoundEngine))
	{
		UE_LOG(LogWwiseFileHandler, Log, TEXT("FWwiseStreamedMediaFileState::CloseFile %" PRIu32 " (%s): Failed closing prefetch without a SoundEngine. Leaking."), MediaId, *DebugName.ToString());
		pMediaMemory = nullptr;
		uMediaSize = 0;
		return CloseFileDone(MoveTemp(InCallback));
	}

	const auto Result = SoundEngine->TryUnsetMedia(this, 1, nullptr);
	if (UNLIKELY(Result == AK_ResourceInUse))
	{
		return CloseFileDefer(MoveTemp(InCallback));
	}
	else
	{
		if (LIKELY(Result == AK_Success))
		{
			UE_LOG(LogWwiseFileHandler, Verbose, TEXT("FWwiseStreamedMediaFileState::CloseFile %" PRIu32 " (%s): Unloaded prefetch."), MediaId, *DebugName.ToString());
			DeallocateMemory(pMediaMemory, uMediaSize, bDeviceMemory, MemoryAlignment, true);
			DEC_MEMORY_STAT_BY(STAT_WwiseFileHandlerPrefetchMemoryAllocated, uMediaSize);
		}
		else
		{
			UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::CloseFile %" PRIu32 " (%s): TryUnsetMedia of prefetch failed: %d (%s). Leaking."), MediaId, *DebugName.ToString(), Result, AkUnrealHelper::GetResultString(Result));
		}
		pMediaMemory = nullptr;
		uMediaSize = 0;
		return CloseFileDone(MoveTemp(InCallback));
	}
}

bool FWwiseStreamedMediaFileState::CanProcessFileOp() const
{
	if (UNLIKELY(State != EState::Loaded))
	{
		UE_LOG(LogWwiseFileHandler, Error, TEXT("FWwiseStreamedMediaFileState::CanProcessFileOp %" PRIu32 " (%s): IO Hook asked for a file operation, but state is not ready."), MediaId, *DebugName.ToString());
		return false;
	}
	return true;
}

AKRESULT FWwiseStreamedMediaFileState::ProcessRead(
	AkFileDesc& InFileDesc, const AkIoHeuristics& InHeuristics,
	AkAsyncIOTransferInfo& OutTransferInfo, FWwiseAkFileOperationDone&& InFileOpDoneCallback)
{
	UE_LOG(LogWwiseFileHandler, VeryVerbose, TEXT("FWwiseStreamedMediaFileState::ProcessRead: Reading %" PRIu32 " bytes @ %" PRIu64 " in file %" PRIu32 " (%s)"),
		OutTransferInfo.uRequestedSize, OutTransferInfo.uFilePosition, MediaId, *DebugName.ToString());

	StreamedFile->ReadAkData(InHeuristics, OutTransferInfo, MoveTemp(InFileOpDoneCallback));
	return AK_Success;
}

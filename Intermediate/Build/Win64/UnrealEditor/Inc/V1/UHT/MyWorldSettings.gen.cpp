// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "V1/Public/MyWorldSettings.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeMyWorldSettings() {}
// Cross Module References
	ENGINE_API UClass* Z_Construct_UClass_AWorldSettings();
	UPackage* Z_Construct_UPackage__Script_V1();
	V1_API UClass* Z_Construct_UClass_AMyWorldSettings();
	V1_API UClass* Z_Construct_UClass_AMyWorldSettings_NoRegister();
// End Cross Module References
	void AMyWorldSettings::StaticRegisterNativesAMyWorldSettings()
	{
	}
	IMPLEMENT_CLASS_NO_AUTO_REGISTRATION(AMyWorldSettings);
	UClass* Z_Construct_UClass_AMyWorldSettings_NoRegister()
	{
		return AMyWorldSettings::StaticClass();
	}
	struct Z_Construct_UClass_AMyWorldSettings_Statics
	{
		static UObject* (*const DependentSingletons[])();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Class_MetaDataParams[];
#endif
		static const FCppClassTypeInfoStatic StaticCppClassTypeInfo;
		static const UECodeGen_Private::FClassParams ClassParams;
	};
	UObject* (*const Z_Construct_UClass_AMyWorldSettings_Statics::DependentSingletons[])() = {
		(UObject* (*)())Z_Construct_UClass_AWorldSettings,
		(UObject* (*)())Z_Construct_UPackage__Script_V1,
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UClass_AMyWorldSettings_Statics::Class_MetaDataParams[] = {
		{ "Comment", "/**\n * \n */" },
		{ "HideCategories", "Actor Advanced Display Events Object Attachment Info Input Blueprint Layers Tags Replication Input Movement Collision Transformation HLOD DataLayers" },
		{ "IncludePath", "MyWorldSettings.h" },
		{ "ModuleRelativePath", "Public/MyWorldSettings.h" },
		{ "ShowCategories", "Input|MouseInput Input|TouchInput" },
	};
#endif
	const FCppClassTypeInfoStatic Z_Construct_UClass_AMyWorldSettings_Statics::StaticCppClassTypeInfo = {
		TCppClassTypeTraits<AMyWorldSettings>::IsAbstract,
	};
	const UECodeGen_Private::FClassParams Z_Construct_UClass_AMyWorldSettings_Statics::ClassParams = {
		&AMyWorldSettings::StaticClass,
		"game",
		&StaticCppClassTypeInfo,
		DependentSingletons,
		nullptr,
		nullptr,
		nullptr,
		UE_ARRAY_COUNT(DependentSingletons),
		0,
		0,
		0,
		0x009002A4u,
		METADATA_PARAMS(Z_Construct_UClass_AMyWorldSettings_Statics::Class_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UClass_AMyWorldSettings_Statics::Class_MetaDataParams))
	};
	UClass* Z_Construct_UClass_AMyWorldSettings()
	{
		if (!Z_Registration_Info_UClass_AMyWorldSettings.OuterSingleton)
		{
			UECodeGen_Private::ConstructUClass(Z_Registration_Info_UClass_AMyWorldSettings.OuterSingleton, Z_Construct_UClass_AMyWorldSettings_Statics::ClassParams);
		}
		return Z_Registration_Info_UClass_AMyWorldSettings.OuterSingleton;
	}
	template<> V1_API UClass* StaticClass<AMyWorldSettings>()
	{
		return AMyWorldSettings::StaticClass();
	}
	AMyWorldSettings::AMyWorldSettings(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {}
	DEFINE_VTABLE_PTR_HELPER_CTOR(AMyWorldSettings);
	AMyWorldSettings::~AMyWorldSettings() {}
	struct Z_CompiledInDeferFile_FID_Users_nwar_OneDrive_Documents_GitHub_GGJ2024_V2_Source_V1_Public_MyWorldSettings_h_Statics
	{
		static const FClassRegisterCompiledInInfo ClassInfo[];
	};
	const FClassRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_nwar_OneDrive_Documents_GitHub_GGJ2024_V2_Source_V1_Public_MyWorldSettings_h_Statics::ClassInfo[] = {
		{ Z_Construct_UClass_AMyWorldSettings, AMyWorldSettings::StaticClass, TEXT("AMyWorldSettings"), &Z_Registration_Info_UClass_AMyWorldSettings, CONSTRUCT_RELOAD_VERSION_INFO(FClassReloadVersionInfo, sizeof(AMyWorldSettings), 3880493955U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_Users_nwar_OneDrive_Documents_GitHub_GGJ2024_V2_Source_V1_Public_MyWorldSettings_h_1369687892(TEXT("/Script/V1"),
		Z_CompiledInDeferFile_FID_Users_nwar_OneDrive_Documents_GitHub_GGJ2024_V2_Source_V1_Public_MyWorldSettings_h_Statics::ClassInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_Users_nwar_OneDrive_Documents_GitHub_GGJ2024_V2_Source_V1_Public_MyWorldSettings_h_Statics::ClassInfo),
		nullptr, 0,
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS

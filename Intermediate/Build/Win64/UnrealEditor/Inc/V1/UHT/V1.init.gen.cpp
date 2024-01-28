// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeV1_init() {}
	static FPackageRegistrationInfo Z_Registration_Info_UPackage__Script_V1;
	FORCENOINLINE UPackage* Z_Construct_UPackage__Script_V1()
	{
		if (!Z_Registration_Info_UPackage__Script_V1.OuterSingleton)
		{
			static const UECodeGen_Private::FPackageParams PackageParams = {
				"/Script/V1",
				nullptr,
				0,
				PKG_CompiledIn | 0x00000000,
				0x29BA454C,
				0xAE0B287B,
				METADATA_PARAMS(nullptr, 0)
			};
			UECodeGen_Private::ConstructUPackage(Z_Registration_Info_UPackage__Script_V1.OuterSingleton, PackageParams);
		}
		return Z_Registration_Info_UPackage__Script_V1.OuterSingleton;
	}
	static FRegisterCompiledInInfo Z_CompiledInDeferPackage_UPackage__Script_V1(Z_Construct_UPackage__Script_V1, TEXT("/Script/V1"), Z_Registration_Info_UPackage__Script_V1, CONSTRUCT_RELOAD_VERSION_INFO(FPackageReloadVersionInfo, 0x29BA454C, 0xAE0B287B));
PRAGMA_ENABLE_DEPRECATION_WARNINGS

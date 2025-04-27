#pragma once

HString GetModuleFilePath(const HString& modulePath, const HString* refModulePath = nullptr);

HString GetMainBinaryFilePath();

HString GetIntermediateModuleFile(const HString& moduleName);

x_uint64 GetFileLastTime(const HString& filePath);

bool FileExist(const HString& filePath);
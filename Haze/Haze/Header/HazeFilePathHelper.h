#pragma once

STDString GetModuleFilePath(const STDString& modulePath, const STDString* refModulePath = nullptr);

STDString GetMainBinaryFilePath();

STDString GetIntermediateModuleFile(const STDString& moduleName);

x_uint64 GetFileLastTime(const STDString& filePath);

bool FileExist(const STDString& filePath);
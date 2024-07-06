#pragma once

HString GetModuleFilePath(const HString& moduleName, const HString* refModulePath = nullptr, const HString* dir = nullptr);

HString GetMainBinaryFilePath();

HString GetIntermediateModuleFile(const HString& moduleName);
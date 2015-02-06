
#ifndef EJOY2D_API_H
#define EJOY2D_API_H

#if defined(EJOY2D_BUILD_AS_DLL)
#define EJOY_API __declspec(dllexport)
#else
#define EJOY_API
#endif

#endif
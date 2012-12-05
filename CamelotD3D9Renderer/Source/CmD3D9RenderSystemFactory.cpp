#include "CmD3D9RenderSystemFactory.h"

namespace CamelotEngine
{
	RenderSystemPtr D3D9RenderSystemFactory::create()
	{
	#ifdef CM_STATIC_LIB
		HINSTANCE hInst = GetModuleHandle( NULL );
	#else
		HINSTANCE hInst = GetModuleHandle( "CamelotD3D9RenderSystem.dll" );
	#endif

		return RenderSystemPtr(new D3D9RenderSystem(hInst));
	}

	D3D9RenderSystemFactory::InitOnStart D3D9RenderSystemFactory::initOnStart;
}
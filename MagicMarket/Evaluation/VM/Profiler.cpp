#include "Statistics.h"

#include "Market.h"
#include <SimpleIni.h>
#include "Profiler.h"

#include <sstream>


namespace MM
{
	namespace vm
	{

		Profiler::Profiler()
		{
		}


		Profiler::~Profiler()
		{
		}

		void Profiler::init(void *_ini)
		{
			const CSimpleIniA &ini = *(CSimpleIniA*)_ini;
		}
	};
};
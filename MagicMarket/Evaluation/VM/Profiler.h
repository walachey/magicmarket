#pragma once

#include <string>
#include <map>
#include <vector>

#include <functional>
#include <fstream>

namespace MM
{
	namespace vm
	{
		class Profiler
		{
		public:
			Profiler();
			~Profiler();

			void log();
			void init(void *ini);
		private:
		};
	}
};

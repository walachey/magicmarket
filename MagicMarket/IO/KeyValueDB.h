#pragma once

#include <string>
#include <map>

namespace MM {

	namespace io
	{
		class KeyValueDB;

		struct Entry
		{
		public:
			Entry(std::string data) : data(data) {}
			std::string data;
		private:
			std::streamoff offset;
			std::size_t length;

			friend class KeyValueDB;
		};

		class KeyValueDB
		{
		public:
			KeyValueDB(std::string filename);
			~KeyValueDB();

			std::string get(std::string key);
			std::string get(std::string category, std::string key);

			void put(std::string key, std::string data);
			void put(std::string category, std::string key, std::string data);

			bool good() { return isGood && loaded; }

		private:
			std::string filename;
			std::map<std::string, Entry> entries;

			void load();
			void assertLoaded();
			bool loaded;
			bool isGood;
		};

	};
};
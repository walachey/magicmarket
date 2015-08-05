#include "KeyValueDB.h"
#include <algorithm>
#include <iostream>
#include <fstream>

namespace MM
{
	namespace io
	{

		KeyValueDB::KeyValueDB(std::string filename) : filename(filename), loaded(false)
		{
		}


		KeyValueDB::~KeyValueDB()
		{
		}

		void KeyValueDB::assertLoaded()
		{
			if (loaded) return;
			load();
			loaded = true;
		}

		void KeyValueDB::load()
		{
			isGood = false;
			std::fstream input(filename.c_str(), std::ios_base::in);
			if (!input.good()) return;

			std::string line;
			while (std::getline(input, line))
			{
				if (line.empty()) continue;
				std::size_t separator = line.find_first_of('\t');
				if (separator == std::string::npos) continue;

				entries.emplace(line.substr(0, separator), line.substr(separator + 1));
			}

			isGood = true;
		}

		std::string KeyValueDB::get(std::string category, std::string key)
		{
			return get(category + "##" + key);
		}

		void KeyValueDB::put(std::string category, std::string key, std::string data)
		{
			return put(category + "##" + key, data);
		}

		std::string KeyValueDB::get(std::string key)
		{
			if (!entries.count(key)) return "";
			return entries.at(key).data;
		}

		void KeyValueDB::put(std::string key, std::string value)
		{
			std::fstream out(filename.c_str(), std::ios_base::out | std::ios_base::app);
			if (!out.good())
			{
				isGood = false;
				return;
			}

			// first time write?
			if (!entries.count(key))
			{
				auto &newEntryIterator = entries.emplace(key, value);
				if (!newEntryIterator.second) return;

				newEntryIterator.first->second.offset = out.tellp();
				out << key << "\t" << value << std::endl;
				newEntryIterator.first->second.length = static_cast<std::size_t>(out.tellp() - newEntryIterator.first->second.offset);
			}
			else
			{
				// update entry
				Entry &entry = entries.at(key);
				const bool lengthChanged = entry.data.size() == value.size();
				entry.data = value;

				out.seekp(entry.offset, std::ios_base::beg);
				
				// try simple replacement
				if (!lengthChanged)
				{
					out << key << "\t" << entry.data << std::endl;
				}
				else // .. write all entries anew
				{
					auto iter = entries.begin();
					while (
						(iter = std::find_if(iter, entries.end(), [&entry](const std::pair<std::string, Entry>& candidate)
							{
								return candidate.second.offset >= entry.offset;
							})
						) != entries.end())
					{
						Entry &entry = iter->second;
						entry.offset = out.tellp();
						out << iter->first << "\t" << entry.data << std::endl;
						entry.length = static_cast<size_t>(out.tellp() - entry.offset);
					}
				}
			}
		}
	};
};
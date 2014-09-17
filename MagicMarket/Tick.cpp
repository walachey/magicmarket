#include "Tick.h"
#include <inttypes.h>

#include <iostream>
#include <sstream>
#include <chrono>
#include <cassert>

namespace MM
{

	Tick::Tick()
	{
	}


	Tick::~Tick()
	{
	}

	std::string Tick::toString()
	{
		std::ostringstream out;
		out << "[" << getBid() << ", " << getAsk() << " @ " << std::ctime(getTimeP()) << "]";
		return out.str();
	}

	
	int Tick::getOutputBitSize()
	{
		return sizeof(uint8_t)+sizeof(time)+sizeof(bid)+sizeof(ask);
	}

	// this expects a binary stream
	std::ostream& operator<< (std::ostream &out, MM::Tick &tick)
	{
		// this is the version of the save format,
		// is is possible that the format changes; this is for forward-compatibility
		uint8_t version = 1;

		out.write((const char*)&version, sizeof(version));
		out.write((const char*)&tick.time, sizeof(tick.time));
		out.write((const char*)&tick.bid, sizeof(tick.bid));
		out.write((const char*)&tick.ask, sizeof(tick.ask));
		
		return out;
	}

	// this expects a binary stream
	std::istream& operator>> (std::istream &in, MM::Tick &tick)
	{
		uint8_t version;
		in.read((char*)&version, sizeof(version));
		

		if (version == 1)
		{
			in.read((char*)&tick.time, sizeof(tick.time));
			in.read((char*)&tick.bid, sizeof(tick.bid));
			in.read((char*)&tick.ask, sizeof(tick.ask));
		}
		return in;
	}
};


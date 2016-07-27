/*
    Copyright (c) 2007-2014 Contributors as noted in the AUTHORS file

    This file is part of 0MQ.

    0MQ is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    0MQ is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <zmq.h>
#include <zmq_utils.h>

#include <stdio.h>
#include <cassert>
#include <stdlib.h>
#include <stdint.h>

#include "EnvironmentVariables.h"
#include "Market.h"
#include "Statistics.h"
#include "VirtualMarket.h"
#include "Interfaces/MTInterface.h"
#include "Trade.h"
#include "Stock.h"
#include "TradingDay.h"

#include <SimpleIni.h>

int main (int argc, char *argv [])
{
	CSimpleIniA ini;
	ini.LoadFile("market.ini");

	environmentVariables.init(&ini);
	market.init(&ini);
	statistics.init(&ini);
	metatrader.init(&ini);
	MM::VirtualMarket::checkInit();

	market.run();
    return 0;
}

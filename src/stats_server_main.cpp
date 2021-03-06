/*
	Copyright (C) 2003-2013 by David White <davewx7@gmail.com>
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <deque>
#include <iostream>
#include <string>
#include <vector>

#include "asserts.hpp"
#include "filesystem.hpp"
#include "json_parser.hpp"
#include "stats_server.hpp"
#include "stats_web_server.hpp"
#include "unit_test.hpp"

COMMAND_LINE_UTILITY(stats_server)
{
	std::string fname = "stats-1.json";
	int port = 5000;

	std::deque<std::string> arguments(args.begin(), args.end());

	while(!arguments.empty()) {
		std::string arg = arguments.front();
		arguments.pop_front();
		if(arg == "-p" || arg == "--port") {
			if(arguments.empty()) {
				std::cerr << "ERROR: " << arg << " specified without port\n";
				return;
			}

			arg = arguments.front();
			arguments.pop_front();

			port = atoi(arg.c_str());
		} else if(arg == "--file") {
			if(arguments.empty()) {
				std::cerr << "ERROR: " << arg << " specified without filename\n";
				return;
			}

			fname = arguments.front();
			arguments.pop_front();

			if(!sys::file_exists(fname)) {
				std::cerr << "COULD NOT OPEN " << fname << "\n";
				return;
			}
		} else {
			std::cerr << "ERROR: UNRECOGNIZED ARGUMENT: '" << arg << "'\n";
			return;
		}
	}

	if(sys::file_exists("stats-definitions.json")) {
		init_tables(json::parse_from_file("stats-definitions.json"));
	} else { 
		init_tables(json::parse_from_file("data/stats-server.json"));
	}

	if(sys::file_exists(fname)) {
		std::cerr << "READING STATS FROM " << fname << "\n";
		read_stats(json::parse_from_file(fname));
		std::cerr << "FINISHED READING STATS FROM " << fname << "\n";
	}

	//Make it so asserts don't make the server die, they throw an
	//exception instead.
	const assert_recover_scope recovery_scope;

	boost::asio::io_service io_service;
	web_server ws(io_service, port);
	io_service.run();
}

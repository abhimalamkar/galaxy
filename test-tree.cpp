/**
 * Copyright (C) 2018 Greenweaves Software Limited
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>
 */
 
#include <vector>
#include "catch.hpp"
#include "treecode.h"

TEST_CASE( "Tree Tests", "[tree]" ) {
	REQUIRE(Node::_count==0);
	
	
	SECTION("Trivial Tree Insert") {
		std::vector<Particle*> particles;
		particles.push_back(new Particle(-1,-1,-1,0,0,0,0));
		particles.push_back(new Particle(-1,-1,1,0,0,0,0));
		Node * tree = Node::create(particles);
		REQUIRE(Node::_count==9);
		delete tree;
	}
	
	SECTION("Larger Tree Insert") {
		std::vector<Particle*> particles;
		particles.push_back(new Particle(-1, -1, -1,0,0,0,1));
		particles.push_back(new Particle(-1, -1, +1,0,0,0,1));
		particles.push_back(new Particle(-1, +1, -1,0,0,0,1));
		particles.push_back(new Particle(+1, -1, -1,0,0,0,1));
		particles.push_back(new Particle(-1, +1, +1,0,0,0,1));
		particles.push_back(new Particle(+1, -1, +1,0,0,0,1));
		particles.push_back(new Particle(+1, +1, -1,0,0,0,1));
		particles.push_back(new Particle(+1, +1, +1,0,0,0,1));
		Node * tree = Node::create(particles);
		REQUIRE(Node::_count==9);
		delete tree;
	}
	
	REQUIRE(Node::_count==0);
}

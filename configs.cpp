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
 
#include <sstream>
#include <stdexcept>
#include <cmath>

#include "configs.h"
#include "physics.h"
#include "spdlog/spdlog.h"


namespace spd = spdlog;

const double pi=4*atan(1);

double M=1;
double a=0.01;

Configuration::Configuration() 
 : 	uniform_distribution_theta(std::uniform_real_distribution<double>(-1,1)),
	uniform_distribution_phi(std::uniform_real_distribution<double>(0,pi)),
	uniform_distribution_radius(std::uniform_real_distribution<double>(0.02,1)),  // FIXME
	uniform_distribution_x(std::uniform_real_distribution<double>(0,1)),
	uniform_distribution_y(std::uniform_real_distribution<double>(0,0.1)){}
	
 /**
  * Create all bodies needed at start of run
  */
 std::vector<Particle*>  Configuration::createParticles( ){
	 switch(model) {
		case Configuration::Plummer:
			return createPlummerDistribution();
		default:
			std::stringstream message;
			message<<__FILE__ <<", " <<__LINE__<<" Invalid model "<<std::endl; 
			throw std::logic_error(message.str().c_str());
	 }
 }	 



/**
 *   Create particles satisfying Plummer distribution, following the derivateion in Hut & Makino
 *   http://www.artcompsci.org/kali/vol/plummer/volume9.pdf
 */
std::vector<Particle*>  Configuration::createPlummerDistribution( ){
	std::vector<Particle*> product;

	for (int i=0;i<numbodies;i++) {
		const double radius=ini_radius/(std::sqrt(std::pow(uniform_distribution_radius(generator),-2.0/3.0)));
        double x; double y; double z;
		randomize_theta_phi(radius,x,y,z);
	
		double vx; double vy;double vz  ;
		randomize_theta_phi(sample_velocity(radius),vx,vy,vz);

        product.push_back( new Particle( x, y, z, vx, vy,vz, mass/numbodies) );
    }

	zero_centre_mass_and_linear_momentum(product,0);
	spdlog::get("galaxy")->info("{0} {1}: initialized {2} bodies.",__FILE__,__LINE__,numbodies);
	return product;
}

/**
 * Sample velocities, ensuring that the initial velocity of any
 * star does not exceed escape velocity
 */
double Configuration::sample_velocity(const double radius) {
	double x=0;
	double y=0.1;
	while (y > sqr(x)*std::pow(1.0-sqr(x),3.5)){
		x=uniform_distribution_x(generator);
		y=uniform_distribution_y(generator);
	}
	return  x * std::sqrt(2.0) * std::pow( 1.0 + sqr(radius),-0.25);
}

/**
 * Convert a scalar, r, into a vector with the same length, and a random orientation
 */
void Configuration::randomize_theta_phi(const double r,double & x,double & y,double& z) {
	const double acos_theta   = uniform_distribution_theta(generator);
	const double theta        = std::acos(acos_theta);
	const double phi          = uniform_distribution_phi(generator);
	x      = r * std::sin(theta)*std::cos(phi);
	y      = r * std::sin(theta)*std::sin(phi);
	z      = r * acos_theta;
}

	

 
/**
 * Set centre of mass and total linear momentum to (0,0,0) by adjusting
 * the position and velocity of each point
 */
 void Configuration::zero_centre_mass_and_linear_momentum(std::vector<Particle*> particles, int iter) {
	if (needToZero==0) return;
	if (needToZero==1 and iter>0) return;
	double x0,y0,z0;
	get_centre_of_mass(particles,x0,y0,z0);

	double total_mass=0;
	for (std::vector<Particle*>::iterator it = particles.begin() ; it != particles.end(); ++it) {
		double x,y,z;
		(*it)->getPos(x,y,z);
		x-=x0;y-=y0;z-=z0;
		(*it)->setPos(x,y,z);
		total_mass+=(*it)->getMass();
	}
	
	double px0,py0,pz0;
	get_momentum(particles,px0,py0,pz0);
	for (std::vector<Particle*>::iterator it = particles.begin() ; it != particles.end(); ++it) {	
		double vx,vy,vz;
		(*it)->getVel(vx,vy,vz);
		vx-=px0/total_mass;vy-=py0/total_mass;vz-=pz0/total_mass;
		(*it)->setVel(vx,vy,vz);
	}
 }
 
 int Configuration::get_max_digits_config(){
	int max_digits_config=5;
	const int max_imgs=std::ceil(((double)max_iter)/img_iter);
	return std::max((int)std::ceil(std::log10(max_imgs)),max_digits_config);
 }
 
 
 /**
  * Save configuration so it can be restarted later
  */
 void  Configuration::save_config( std::vector<Particle*>& bodies, int iter) {
	std::stringstream file_name;
    file_name << path<< config_file_name;
	backup(file_name.str().c_str());
    std::ofstream ofile(file_name.str().c_str());
	ofile << "Version="<<config_version<<"\n";
	ofile << "iteration=" << iter  << "\n";
	ofile << "theta=" << encode(theta)  << "\n";
	ofile << "G=" << encode(G)  << "\n";
	ofile << "dt=" << encode(dt)  << "\n";
    for (unsigned i=0; i<bodies.size(); ++i) {
		double px, py, pz;
		bodies[i] -> getPos(px, py,pz);
		double vx, vy,vz;
		bodies[i] -> getVel(vx, vy,vz);
		double m=bodies[i]->getMass();
		ofile <<i<<","<< encode(px)<<","<< encode(py)<<","<< encode(pz)<<","<< encode(m) <<","<< encode(vx)<<","<< encode(vy)<<","<< encode(vz)<<"\n";
	}
	ofile << "End\n";
}

/**
  * Restore configuration from saved file
  */
bool Configuration::restore_config(std::vector<Particle*>& particles,int& iter) {
	auto logger=spdlog::get("galaxy");
	std::stringstream file_name;
    file_name << path<< config_file_name;
	std::ifstream config_file(file_name.str().c_str());

    if(! config_file.is_open())
        return false;
	enum State{expect_version, expect_iteration, expect_theta, expect_g, expect_dt, expect_body, expect_eof};
	State state=State::expect_version;
	while(! config_file.eof())   {
		std::string line;
        getline(config_file,line);
        std::stringstream ss(line);
		std::string token;
		switch(state) {
			case State::expect_version:
				token = line.substr(1+line.find("="));
				logger->info("Version {0}",token);
				state=State::expect_iteration;
				break;
			case State::expect_iteration:
				token = line.substr(1+line.find("="));
				iter=atoi(token.c_str());
				logger->info("Iter = {0}",iter);
				state=State::expect_theta;
				break;
			case State::expect_theta:
				token = line.substr(1+line.find("="));
				theta=decode(token);
				logger->info("Theta={0}",theta);
				state=State::expect_g;
				break;
			case State::expect_g:
				token = line.substr(1+line.find("="));
				G=decode(token);
				logger->info("G={0}",G);
				state=State::expect_dt;
				break;
			case State::expect_dt:
				token = line.substr(1+line.find("="));
				dt=decode(token);
				logger->info("dt={0}",dt);
				state=State::expect_body;
				break;
			case State::expect_body:
				if (line.find("End")==0)
					state=State::expect_eof;
				else
					particles.push_back(extract_particle(line));
				break;
			case State::expect_eof:
				if (line.length()>0){
					logger->info("Unexpected text following end={0}",line);
					return false;
				}
			default:
				if (line.length()>0){
					logger->info("Unexpected state {0}",state);
					return false;
				}
		}
    }
	if (state!=State::expect_eof) {
		std::cout<<"Unexpected state: "<<state<<"-" <<State::expect_eof <<std::endl;
		return false;
	}
	return true;
}

/**
 * Retrieve position, mass, and velocities stored for one Body
 */
Particle * Configuration::extract_particle(std::string line){
	enum State {expect_i,expect_x,expect_y,expect_z,expect_m,expect_vx,expect_vy,expect_vz,end_line};
	State state=expect_i;
	double px, py, pz, m, vx,vy,vz;
	while (state!=end_line) {
		int pos=line.find(",");
		std::string token=pos>=0 ? line.substr(0,pos) : line;
		line=line.substr(pos+1);
	
		switch (state){
			case expect_i:
				state=expect_x;
				break;
			case expect_x:
				state=expect_y;
				px=decode(token);
				break;
			case expect_y:
				state=expect_z;
				py=decode(token);
				break;
			case expect_z:
				state=expect_m;
				pz=decode(token);
				break;
			case expect_m:
				state=expect_vx;
				m=decode(token);
				break;
			case expect_vx:
				vx=decode(token);
				state=expect_vy;
				break;
			case expect_vy:
				vy=decode(token);
				state=expect_vz;
				break;
			case expect_vz:
				vz=decode(token);
				state=end_line;
		}
	}
	
	return new Particle(px,py,pz,vx,vy,vz,m);
}

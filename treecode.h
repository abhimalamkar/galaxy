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
 
#ifndef _TREECODE_H
#define _TREECODE_H

#include <vector>

#include "particle.h"

/**
 *  Represents one node in Barnes Hut Octree
 */
class Node {
  public:
  
	/**
	 * Create an oct-tree from a set of particles
	 */
	static Node * create(std::vector<Particle*>& particles);
	  /**
	   * Number of nodes allocated: used in testing
	   */
	  static int _count;
	  
	  /**
	   *  Used to traverse tree.
	   */
	  class Visitor {
		public:
			enum Status{Stop, Continue, Sideways};
			virtual Status visit(Node * node)=0;
			virtual void propagate(Node * node,Node * child){;}
			virtual bool depart(Node * node) {return true;};
	  };
	  
	 /**
	  * Indicates type of node. External Nodes use the index of the
      * associated particle instead of one of these values.
	  */
  	enum Status {Internal=-2, Unused=-1};
	
	/**
	 *   Used to ensure we have an octree
	 */
	enum {N_Children=8};
	 
	/**
	 *  Create one node for tree
	 */
    Node(double xmin,double xmax,double ymin,double ymax,double zmin,double zmax);
	
	/**
	 * Insert one particle in tree
	 */
	void insert(int new_particle_index,std::vector<Particle*>& particles);
	
	/**
	 * Destroy node and its descendants.
	 */
	virtual ~Node();
	
	/**
	 * Used to traverse tree
	 */
	bool visit(Visitor& visitor);
	
	/**
	 * Indicates type of node. External Nodes use the index of the
     * associated particle instead of one of these values.
	 */
	int getStatus() { return _particle_index;}
	
	/**
	 * Get mass and centre of mass
	 */
	void getPhysics(double& m, double& x, double& y, double &z) {m=_m;x=_x;y=_y;z=_z;}
	
	/**
	 * Set mass and centre of mass
	 */
	void setPhysics(double m, double x, double y, double z) {_m=m,_x=x;_y=y;_z=z;}
	
	/**
	 *   Used to calculate centre of mass for internal nodes.
	 */
	void accumulatePhysics(Node* other);

	double getSide() {return _xmax - _xmin;}
	
	/**
	 * Bounding box for Node. This will be subdivided as we move down the tree
	 */
	const double _xmin, _xmax, _ymin, _ymax, _zmin, _zmax, _xmean, _ymean, _zmean;
		
	/**
	 * Determine the bounding box for set of particles. Make it slightly 
	 * larger than strictly needed, so everything is guaranteed to be inside box
	 */
	static get_limits(std::vector<Particle*>& particles,
				double& xmin,double& xmax,
				double& ymin,double& ymax,
				double& zmin,double& zmax,
				const double epsilon=0.0001);
	
  private:
	
	/**
	 * Used to map a triple to an octet
	 */
	int _get_child_index(int i, int j, int k) {return 4*i+2*j+k;}
	
	/**
	 * Find correct subtree to store particle, using bounding rectangular box
	 */
	int _get_child_index(Particle * particle);
	
	/**
	 * Used when we have just split an External node, but the incumbent and new
	 * node both want to occupy the same child.
	 */
	void _pass_down(int particle_index,int incumbent,std::vector<Particle*>& particles);
	
	/**
	 * Used when we have just split an External node, so we need to pass
	 * the incumbent and a new particle down the tree
	 */
	void _insert_or_propagate(int particle_index,int incumbent,std::vector<Particle*>& particles);
	
	/**
	 * Convert an External Node into an Internal one, and
	 * determine bounding boxes for children, so we can 
	 * Propagate particle down
	 */
	void _split_node();
	
	/**
	 * Indicates type of node. External Nodes use the index of the
     * associated particle instead of one of these values.
	 */
	int _particle_index;
	
	/**
	 * Descendants of this node - only for an Internal Node
	 */
	Node * _child[N_Children];
	
	/**
	 *  Mass and centre of mass
	 */
	double _m, _x, _y, _z;
};



#endif

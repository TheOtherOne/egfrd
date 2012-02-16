#!/usr/bin/python

# Small script to test transfers between membranes
#
# Make sure that the egfrd system is added to your PYTHONPATH
# This means, in bash for example:
# $ export PYTHONPATH=$HOME/egfrd

# Modules
# ===============================
import sys
import os
import datetime

from egfrd import *
from visualization import vtklogger

import model
import gfrdbase
import _gfrd


# Unique seed
# ===============================
currenttime = (long(
datetime.datetime.now().month*30.5*24*3600*1e6+
datetime.datetime.now().day*24*3600*1e6+datetime.datetime.now().hour*3600*1e6+
datetime.datetime.now().minute*60*1e6+datetime.datetime.now().second*1e6+
datetime.datetime.now().microsecond))
myrandom.seed(currenttime)
print "(Seed " + str(currenttime) + ")"
 

# Constants
# ===============================
#logging = False
logging = True
sigma = 1e-9        # Diameter particle
DC = 1e-13           # Diffusion constant
N = 20000           # Number of steps simulation will last
world_size = 1e-6   # Lengths of simulation box
Nparticles = 1     # Number of particles in the simulation
k = 2e-14           # Reaction constant

# VTK Logger
# ===============================
if (logging == True):
    vtk_output_directory = 'VTK_out'
    if (os.path.exists(vtk_output_directory)):
        print '** WARNING: VTK output directory already exists, possible '+ \
                'present old VTK files might lead to errors.'


# Basic set up simulator
# ===============================
# Model
m = model.ParticleModel(2*world_size)

# Planes ("membranes")
membrane1_type = _gfrd.StructureType()
membrane1_type['name'] = 'membrane1_type'
membrane2_type = _gfrd.StructureType()
membrane2_type['name'] = 'membrane2_type'
membrane3_type = _gfrd.StructureType()
membrane3_type['name'] = 'membrane3_type'
membrane4_type = _gfrd.StructureType()
membrane4_type['name'] = 'membrane4_type'
membrane5_type = _gfrd.StructureType()
membrane5_type['name'] = 'membrane5_type'
membrane6_type = _gfrd.StructureType()
membrane6_type['name'] = 'membrane6_type'

m.add_structure_type(membrane1_type)
m.add_structure_type(membrane2_type)
m.add_structure_type(membrane3_type)
m.add_structure_type(membrane4_type)
m.add_structure_type(membrane5_type)
m.add_structure_type(membrane6_type)

# Species
A = model.Species('A', DC, sigma, membrane1_type['name'])
m.add_species_type(A)
B = model.Species('B', DC, sigma, membrane2_type['name'])
m.add_species_type(B)
C = model.Species('C', DC, sigma, membrane3_type['name'])
m.add_species_type(C)
D = model.Species('D', DC, sigma, membrane4_type['name'])
m.add_species_type(D)
E = model.Species('E', DC, sigma, membrane5_type['name'])
m.add_species_type(E)
F = model.Species('F', DC, sigma, membrane6_type['name'])
m.add_species_type(F)

# Reaction rules
rAB = model.create_binding_reaction_rule(A, membrane2_type, B, k)
m.network_rules.add_reaction_rule(rAB)
rBA = model.create_unbinding_reaction_rule(B, membrane1_type, A, k)
m.network_rules.add_reaction_rule(rBA)

rBC = model.create_binding_reaction_rule(B, membrane3_type, C, k)
m.network_rules.add_reaction_rule(rBC)
rCB = model.create_unbinding_reaction_rule(C, membrane2_type, B, k)
m.network_rules.add_reaction_rule(rCB)

rCD = model.create_binding_reaction_rule(C, membrane4_type, D, k)
m.network_rules.add_reaction_rule(rCD)
rDC = model.create_unbinding_reaction_rule(D, membrane3_type, C, k)
m.network_rules.add_reaction_rule(rDC)

rAD = model.create_binding_reaction_rule(A, membrane4_type, D, k)
m.network_rules.add_reaction_rule(rAD)
rDA = model.create_unbinding_reaction_rule(D, membrane1_type, A, k)
m.network_rules.add_reaction_rule(rDA)

rAE = model.create_binding_reaction_rule(A, membrane5_type, E, k)
m.network_rules.add_reaction_rule(rAE)
rEA = model.create_unbinding_reaction_rule(E, membrane1_type, A, k)
m.network_rules.add_reaction_rule(rEA)

rBE = model.create_binding_reaction_rule(B, membrane5_type, E, k)
m.network_rules.add_reaction_rule(rBE)
rEB = model.create_unbinding_reaction_rule(E, membrane2_type, B, k)
m.network_rules.add_reaction_rule(rEB)

rCE = model.create_binding_reaction_rule(C, membrane5_type, E, k)
m.network_rules.add_reaction_rule(rCE)
rEC = model.create_unbinding_reaction_rule(E, membrane3_type, C, k)
m.network_rules.add_reaction_rule(rEC)

rDE = model.create_binding_reaction_rule(D, membrane5_type, E, k)
m.network_rules.add_reaction_rule(rDE)
rED = model.create_unbinding_reaction_rule(E, membrane4_type, D, k)
m.network_rules.add_reaction_rule(rED)

rAF = model.create_binding_reaction_rule(A, membrane6_type, F, k)
m.network_rules.add_reaction_rule(rAF)
rFA = model.create_unbinding_reaction_rule(F, membrane1_type, A, k)
m.network_rules.add_reaction_rule(rFA)

rBF = model.create_binding_reaction_rule(B, membrane6_type, F, k)
m.network_rules.add_reaction_rule(rBF)
rFB = model.create_unbinding_reaction_rule(F, membrane2_type, B, k)
m.network_rules.add_reaction_rule(rFB)

rCF = model.create_binding_reaction_rule(C, membrane6_type, F, k)
m.network_rules.add_reaction_rule(rCF)
rFC = model.create_unbinding_reaction_rule(F, membrane3_type, C, k)
m.network_rules.add_reaction_rule(rFC)

rDF = model.create_binding_reaction_rule(D, membrane6_type, F, k)
m.network_rules.add_reaction_rule(rDF)
rFD = model.create_unbinding_reaction_rule(F, membrane4_type, D, k)
m.network_rules.add_reaction_rule(rFD)


# Create instances of membrane types
membraneA = _gfrd.create_planar_surface(membrane1_type['name'], [0,0,0], [1,0,0], [0,1,0], world_size, world_size)
membraneA.sid = membrane1_type.id
m.add_structure(membraneA)
membraneB = _gfrd.create_planar_surface(membrane2_type['name'], [0,0,0], [0,0,1], [0,1,0], world_size, world_size)
membraneB.sid = membrane2_type.id
m.add_structure(membraneB)
membraneC = _gfrd.create_planar_surface(membrane3_type['name'], [0,0,world_size], [1,0,0], [0,1,0], world_size, world_size)
membraneC.sid = membrane3_type.id
m.add_structure(membraneC)
membraneD = _gfrd.create_planar_surface(membrane4_type['name'], [world_size,0,0], [0,0,1], [0,1,0], world_size, world_size)
membraneD.sid = membrane4_type.id
m.add_structure(membraneD)
membraneE = _gfrd.create_planar_surface(membrane5_type['name'], [0,0,0], [1,0,0], [0,0,1], world_size, world_size)
membraneE.sid = membrane5_type.id
m.add_structure(membraneE)
membraneF = _gfrd.create_planar_surface(membrane6_type['name'], [0,world_size,0], [1,0,0], [0,0,1], world_size, world_size)
membraneF.sid = membrane6_type.id
m.add_structure(membraneF)


# World
w = gfrdbase.create_world(m, 3)
# Simulator
s = EGFRDSimulator(w, myrandom.rng)

# Set up logger
if (logging == True):
    l = vtklogger.VTKLogger(s, vtk_output_directory, extra_particle_step=True) 

# Throw in particles
place_particle(w, A, [world_size/2, world_size/2, 0])  # place one particle on membraneA
#throw_in_particles(w, A, Nparticles) # THIS DOES NOT WORK BECAUSE PARTICLES INTERSECT WITH PLANES YET!
#throw_in_particles(w, B, Nparticles)

# Running 2
# ===============================
for t in range(N):

    # if logging, log and also use "try statement"
    if (logging == True): 
        # take a step (but ignore errors)
        try:
            s.step()
        except:
            print "Broken (run with logging=False for errormessage)!"
            break
        # log the step
        l.log()
    # otherwise just make steps.
    else:
        s.step()

s.stop(s.t)

if (logging == True):
    l.stop()

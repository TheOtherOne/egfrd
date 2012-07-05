#ifndef PLANAR_SURFACE_HPP
#define PLANAR_SURFACE_HPP

#include <boost/bind.hpp>
#include "Surface.hpp"
#include "Plane.hpp"
#include "freeFunctions.hpp"
#include "StructureFunctions.hpp"

template <typename Tobj_, typename Tid_, typename Ttraits_>
class StructureContainer;


template<typename Ttraits_>
class PlanarSurface
    : public BasicSurfaceImpl<Ttraits_, Plane<typename Ttraits_::length_type> >
{
    // The planar surface is an implementation of a Basic surface parameterized with the plane

public:
    typedef BasicSurfaceImpl<Ttraits_, Plane<typename Ttraits_::length_type> > base_type;
    typedef Ttraits_ traits_type;
    typedef typename base_type::structure_name_type     structure_name_type;        // This is just the name of the structure
    typedef typename base_type::structure_id_type       structure_id_type;
    typedef typename base_type::structure_type_id_type  structure_type_id_type;
    typedef typename base_type::shape_type              shape_type;
    typedef typename base_type::rng_type                rng_type;
    typedef typename base_type::position_type           position_type;
    typedef typename base_type::length_type             length_type;
    typedef typename base_type::side_enum_type          side_enum_type;
    typedef typename traits_type::species_type          species_type;
    typedef typename traits_type::structure_type        structure_type;

    typedef StructureContainer<Structure<traits_type>, structure_id_type, traits_type>    structure_container_type;

    typedef std::pair<position_type, position_type>                               position_pair_type;
    typedef std::pair<position_type, structure_id_type>                           position_structid_pair_type;
    typedef std::pair<position_structid_pair_type, position_structid_pair_type>   position_structid_pair_pair_type;


    /*** Simple structure-specific sampling functions ***/
    // Draw a random position in the plane
    virtual position_type random_position(rng_type& rng) const
    {
        return ::random_position(base_type::shape(), boost::bind(&rng_type::uniform, rng, -1., 1.));
    }

    // Create a random vector in the plane
    virtual position_type random_vector(length_type const& r, rng_type& rng) const
    {
        return multiply(
            normalize(
                add(
                    multiply(
                        base_type::shape().units()[0], rng.uniform(-1., 1.)),
                    multiply(
                        base_type::shape().units()[1], rng.uniform(-1., 1.)))), r);
    }

    // Calculate a bd displacement for particles diffusing in the plane
    virtual position_type bd_displacement(length_type const& mean, length_type const& r, rng_type& rng) const
    {
        length_type const x(rng.normal(0., r)), y(rng.normal(0., r));
        return add(
            multiply(base_type::shape().unit_x(), x),
            multiply(base_type::shape().unit_y(), y));
    }

    /*** New BD scheme functions ***/
    // Rate for binding to particle on the structure
    virtual Real get_1D_rate_geminate( Real const& k, length_type const& r01) const
    {
        return k / (2 * M_PI * r01); 
    }

    // Rate for binding to the structure
    virtual Real get_1D_rate_surface( Real const& k, length_type const& r0 ) const
    {
        return k;
    }

    // Reaction volume for binding to particle in the structure
    virtual Real particle_reaction_volume( length_type const& r01, length_type const& rl ) const
    {
        length_type r01l( r01 + rl );
        length_type r01l_sq( r01l * r01l );
        length_type r01_sq( r01 * r01 );

        return M_PI * ( r01l_sq - r01_sq );
    }
    
    // Reaction volume for binding to the structure
    virtual Real surface_reaction_volume( length_type const& r0, length_type const& rl ) const
    {
        //factor 2 because surface has two possible reaction sides due to cyclic BCn.
        return 2*rl; // FIXME because we reject the above constraint at some point
    }
    
    // Vector of dissociation from the structure to parent structure
    virtual position_type surface_dissociation_vector( rng_type& rng, length_type const& r0, length_type const& rl ) const
    {
        Real X( rng.uniform(0.,1.) );
        length_type diss_vec_length( X*rl );

        return multiply( base_type::shape().unit_z(), (rng.uniform_int(0, 1) * 2 - 1) * diss_vec_length );
    }
    
    // Normed direction of dissociation from the structure to parent structure
    virtual position_type surface_dissociation_unit_vector( rng_type& rng ) const
    {
        return base_type::shape().unit_z();
    }

    // Positions created at dissociation of one particle on the structure into two particles on the structure
    virtual position_pair_type geminate_dissociation_positions( rng_type& rng, species_type const& s0, species_type const& s1, position_type const& op, 
        length_type const& rl ) const
    {
        length_type const r01( s0.radius() + s1.radius() );
        Real const D01( s0.D() + s1.D() );
    
        Real X( rng.uniform(0.,1.) );

        length_type const r01l( r01 + rl );
        length_type const r01l_sq( r01l * r01l );
        length_type const r01_sq( r01 * r01 );
        
        length_type const diss_vec_length( sqrt( X * (r01l_sq - r01_sq) + r01_sq ) );

        position_type const m( random_vector( diss_vec_length, rng ) );

        return position_pair_type( op - m * s0.D() / D01,
                                    op + m * s1.D() / D01 );
    }
    
    // Positions created at dissociation of one particle on the structure into two particles, one of which ends up in the bulk
    virtual position_pair_type special_geminate_dissociation_positions( rng_type& rng, species_type const& s_surf, species_type const& s_bulk, 
        position_type const& op_surf, length_type const& rl ) const
    {
        length_type const r01( s_bulk.radius() + s_surf.radius() );
        Real const D01( s_bulk.D() + s_surf.D() );
        Real const D_bulk_D01( s_bulk.D() / D01 );
        Real const D_surf_D01( s_surf.D() / D01 );
        
        //Real theta_max( M_PI/2 - asin(s_bulk.radius() / ( r01 )) );
        //theta_max = theta_max < 0 ? 0 : theta_max;
        
        Real const theta( rng.uniform(0.,1.) * M_PI );        
        Real const phi( rng.uniform(0.,1.) * 2 * M_PI );
        
        Real const X( rng.uniform(0.,1.) );
        length_type const r01l( r01 + rl );
        length_type const r01l_cb( r01l * r01l * r01l );
        length_type const r01_cb( r01 * r01 * r01 );
        
        length_type const diss_vec_length( cbrt( X * (r01l_cb - r01_cb ) + r01_cb ) );   
        
        position_type unit_z( cross_product( base_type::shape().unit_x(), base_type::shape().unit_y() ) );
        unit_z = normalize ( unit_z );  // TODO unit_z is already defined, don't have to do a cross product
        
        //unit_z = multiply(unit_z, rng.uniform_int(0, 1) * 2 - 1);
        
        length_type const x( diss_vec_length * sin( theta ) * cos( phi ) );
        length_type const y( diss_vec_length * sin( theta ) * sin( phi ) );
        length_type const z( diss_vec_length * cos( theta ) );
        
        position_pair_type pp01;
        
        // This is the particle that will end up on the surface
        pp01.first = subtract( op_surf, 
                       add( base_type::shape().unit_x() * (x * D_surf_D01),
                              base_type::shape().unit_y() * (y * D_surf_D01) ) );
        
        // This is the particle that will end up in the bulk
        pp01.second = add( op_surf, 
                       add( base_type::shape().unit_x() * (x * D_bulk_D01),
                         add( base_type::shape().unit_y() * (y * D_bulk_D01),
                                unit_z * z ) ) );

        return pp01;
        
    }
    
    // Used by newBDPropagator
    virtual length_type newBD_distance(position_type const& new_pos, length_type const& radius, position_type const& old_pos, length_type const& sigma) const
    {
        const boost::array<length_type, 2> half_lengths(base_type::shape().half_extent());
        const boost::array<length_type, 3> new_pos_xyz(::to_internal(base_type::shape(), new_pos));
        const boost::array<length_type, 3> old_pos_xyz(::to_internal(base_type::shape(), old_pos));
        if (new_pos_xyz[2] * old_pos_xyz[2] < 0 &&
            ((abs(new_pos_xyz[0]) < half_lengths[0] && abs(new_pos_xyz[1]) < half_lengths[1]) ||
             (abs(old_pos_xyz[0]) < half_lengths[0] && abs(old_pos_xyz[1]) < half_lengths[1])))
        {
            return -1.0 * base_type::distance(new_pos) + sigma;
        }
        else
        {
            return base_type::distance(new_pos) + sigma;
        }
    }
/*
    virtual length_type minimal_distance(length_type const& radius) const
    {
        // PlanarSurface has thickness of 0.
        return radius * traits_type::MINIMAL_SEPARATION_FACTOR;
    }
*/
    /*** Boundary condition handling ***/
    // FIXME This is a mess but it works. See ParticleContainerBase.hpp for explanation.
    virtual position_structid_pair_type apply_boundary(position_structid_pair_type const& pos_struct_id,
                                                       structure_container_type const& structure_container) const
    {
        return structure_container.apply_boundary(*this, pos_struct_id);
    }

    virtual position_structid_pair_type cyclic_transpose(position_structid_pair_type const& pos_struct_id,
                                                         structure_container_type const& structure_container) const
    {
        return structure_container.cyclic_transpose(*this, pos_struct_id);
    }


    // *** Dynamic dispatch for the structure functions *** //
    // *** 1 *** - One new position
    // This requires a double dynamic dispatch.
    virtual position_structid_pair_type get_pos_sid_pair(structure_type const& target_structure, position_type const& position,
                                                         length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
        return target_structure.get_pos_sid_pair_helper(*this, position, offset, reaction_length, rng);
    }
    // the associated helper function
    template <typename Tstruct_>
    position_structid_pair_type get_pos_sid_pair_helper(Tstruct_ const& origin_structure, position_type const& position,
                                                        length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
        // Dispatch to function with well-defined typing
        return ::get_pos_sid_pair(origin_structure, *this, position, offset, reaction_length, rng);
    }
    
    // *** 2 *** - Two new positions
    // Same principle as above, but different return type
    virtual position_structid_pair_pair_type get_pos_sid_pair_pair(structure_type const& target_structure, position_type const& position,
                                                                   species_type const& s1, species_type const& s2, length_type const& reaction_length, rng_type const& rng) const
    {
        return target_structure.get_pos_sid_pair_pair_helper(*this, position, s1, s2, reaction_length, rng);
    }
    // the associated helper function
    template <typename Tstruct_>
    position_structid_pair_pair_type get_pos_sid_pair_pair_helper(Tstruct_ const& origin_structure, position_type const& position,
                                                                  species_type const& s1, species_type const& s2, length_type const& reaction_length, rng_type const& rng) const
    {
        // Dispatch to function with well-defined typing
        return ::get_pos_sid_pair_pair(origin_structure, *this, position, s1, s2, reaction_length, rng);
    }
    
    // *** 3 *** - Pair reactions => two origin structures => triple switchbox
    // Overloading get_pos_sid_pair with signature (origin_structure2, target_structure_type_id, ...) and
    // introducing another double dynamic dispatch.
    virtual position_structid_pair_type get_pos_sid_pair(structure_type const& origin_structure2, structure_type_id_type const& target_sid,
                                                         position_type const& CoM, length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
        return origin_structure2.get_pos_sid_pair_helper_two_origins(*this, target_sid, CoM, offset, reaction_length, rng);
    }
    // two associated helper function
    template <typename Tstruct_>
    position_structid_pair_type get_pos_sid_pair_helper_two_origins(Tstruct_ const& origin_structure1, structure_type_id_type const& target_sid,
                                                                    position_type const& CoM, length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
          // The types of both origin structures now are determined.
          // As a next step, determine which one is the target structure and call the right
          // structure function with the other one as origin structure.
          
          if( this->structure_type_id() != origin_structure1->structure_type_id() )
          // if the two pair reactants come from different types of structure
          {

              if ( (origin_structure1->structure_id() == this->id) && (origin_structure1->structure_type_id() == target_sid) )
              // *this is the parent of origin_structure1, i.e. target_structure is origin_structure1
              {
                  // Call transition function with *this as origin_structure and origin_structure1 as target_structure
                  return ::get_pos_sid_pair(*this, origin_structure1, CoM, offset, reaction_length, rng);
              }
              else if ( (this->structure_id() == origin_structure1->id) && (this->structure_type_id() == target_sid) )
              // origin_structure1 is the parent of *this, i.e. target_structure is *this (= origin_structure2)
              {
                  // Call transition function with origin_structure1 as origin_structure and *this as target_structure
                  return ::get_pos_sid_pair(origin_structure1, *this, CoM, offset, reaction_length, rng);
              }
              else
              {
                  throw propagation_error("Particles can be at most one hierarchical level apart for a pair reaction.");
                  // TODO In principle this constraint could be dropped; the target structure should be determined
                  // already by comparing structure_type_id's with target_sid (which is fed in from the reaction rules).
              }  
        }
        else
        // the reactants live on the same structure type, i.e. the product will also end up on 
        {
            // Call transition function with origin_structure = target_structure = *this
            return ::get_pos_sid_pair(origin_structure1, *this, CoM, offset, reaction_length, rng);
            // Note that apply_boundary should place the product particle on the right one of the two structures
            // afterwards in case that *this->id != origin_structure1->id (right now we postulate only that the
            // structure_type_id's are the same!).
        }
        
    }
    
    // *** 4 *** - Generalized functions for pair reactions with two origin structures and one target structure
    // NOTE: This is yet unused, but possibly useful in the future.
    // Overloading get_pos_sid_pair again with signature (origin_structure2, target_structure, ...) and introducing
    // a triple dynamic dispatch.
    virtual position_structid_pair_type get_pos_sid_pair(structure_type const& origin_structure2, structure_type const& target_structure, position_type const& position,
                                                         length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
        return origin_structure2.get_pos_sid_pair_helper1(*this, target_structure, position, offset, reaction_length, rng);
    }
    // two associated helper functions this time
    template <typename Tstruct1_>
    position_structid_pair_type get_pos_sid_pair_helper1(Tstruct1_ const& origin_structure1, structure_type const& target_structure, position_type const& position,
                                                         length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
        return target_structure.get_pos_sid_pair_helper2(origin_structure1, *this, position, offset, reaction_length, rng);
    }
    template <typename Tstruct1_, typename Tstruct2_>
    position_structid_pair_type get_pos_sid_pair_helper2(Tstruct1_ const& origin_structure1, Tstruct2_ origin_structure2, position_type const& position,
                                                         length_type const& offset, length_type const& reaction_length, rng_type const& rng) const
    {
        if(origin_structure1.id == *this->id)
            // Dispatch to function with well-defined typing
            return ::get_pos_sid_pair(origin_structure2, *this, position, offset, reaction_length, rng);
        
        else if(origin_structure2.id == *this->id)
            // Dispatch to function with well-defined typing
            return ::get_pos_sid_pair(origin_structure1, *this, position, offset, reaction_length, rng);
        
        else
            throw propagation_error("Target structure must be one of the origin structures for pair reaction.");
    }
    

    /*** Formerly used functions of the Morelli scheme ***/
    // DEPRECATED
    virtual length_type drawR_gbd(Real const& rnd, length_type const& r01, Real const& dt, Real const& D01, Real const& v) const
    {
        //TODO: use the 2D BD function instead of the 3D one - failed on very hard integral.
        return drawR_gbd_3D(rnd, r01, dt, D01);
    }
    // DEPRECATED
    virtual Real p_acceptance(Real const& k_a, Real const& dt, length_type const& r01, position_type const& ipv, 
                                Real const& D0, Real const& D1, Real const& v0, Real const& v1) const
    {
        //TODO: use the 2D BD function instead of the 3D one. - Solution known
        return k_a * dt / ((I_bd_3D(r01, dt, D0) + I_bd_3D(r01, dt, D1)) * 4.0 * M_PI);
    }
    // DEPRECATED
    virtual position_type dissociation_vector( rng_type& rng, length_type const& r01, Real const& dt, 
                                                Real const& D01, Real const& v ) const
    {
        return random_vector( drawR_gbd(rng(), r01, dt, D01, v), rng );
    }

    virtual void accept(ImmutativeStructureVisitor<traits_type> const& visitor) const
    {
        visitor(*this);
    }

    virtual void accept(MutativeStructureVisitor<traits_type> const& visitor)
    {
        visitor(*this);
    }

    PlanarSurface(structure_name_type const& name, structure_type_id_type const& sid, structure_id_type const& parent_struct_id, shape_type const& shape)
        : base_type(name, sid, parent_struct_id, shape) {}
};


#endif /* PLANAR_SURFACE_HPP */

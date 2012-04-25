#ifndef PLANE_HPP
#define PLANE_HPP

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/array.hpp>
#include <boost/multi_array.hpp>
#include <utility>
#include <algorithm>
#include "utils/array_helper.hpp"
#include "Shape.hpp"
#include "linear_algebra.hpp"

template<typename T_>
class Plane
{
public:
    typedef T_ value_type;
    typedef Vector3<T_> position_type;
    typedef T_ length_type;

public:
    Plane(position_type const& position = position_type())
        : position_(position),
          units_(array_gen(
            create_vector<position_type>(1., 0., 0.),
            create_vector<position_type>(0., 1., 0.),
            create_vector<position_type>(0., 0., 1.))),
          half_extent_(array_gen<length_type>(0.5, 0.5)) {}

    template<typename Tarray_>
    Plane(position_type const& position, Tarray_ const& half_extent)
        : position_(position),
          units_(array_gen(
            create_vector<position_type>(1., 0., 0.),
            create_vector<position_type>(0., 1., 0.),
            create_vector<position_type>(0., 0., 1.)))
    {
        std::copy(boost::begin(half_extent), boost::end(half_extent),
                  boost::begin(half_extent_));
    }

    template<typename Tarray1, typename Tarray2>
    Plane(position_type const& position,
        Tarray1 const& units, Tarray2 const& half_extent)
        : position_(position)
    {
        std::copy(boost::begin(units), boost::end(units),
                  boost::begin(units_));
        std::copy(boost::begin(half_extent), boost::end(half_extent),
                  boost::begin(half_extent_));
    }

    template<typename Tarray_>
    Plane(position_type const& position,
        position_type const& vx,
        position_type const& vy,
        Tarray_ const& half_extent = array_gen<length_type>(0.5, 0.5))
        : position_(position), units_(array_gen(vx, vy, cross_product(vx, vy)))
    {
        std::copy(boost::begin(half_extent), boost::end(half_extent),
                  boost::begin(half_extent_));
    }

    Plane(position_type const& position,
        position_type const& vx,
        position_type const& vy,
        length_type const& half_lx,
        length_type const& half_ly)
        : position_(position), units_(array_gen(vx, vy, cross_product(vx, vy))),
          half_extent_(array_gen<length_type>(half_lx, half_ly)) {}

    position_type const& position() const
    {
        return position_;
    }

    position_type& position()
    {
        return position_;
    }

    position_type const& unit_x() const
    {
        return units_[0];
    }

    position_type& unit_x()
    {
        return units_[0];
    }

    position_type const& unit_y() const
    {
        return units_[1];
    }

    position_type& unit_y()
    {
        return units_[1];
    }

    position_type const& unit_z() const
    {
        return units_[2];
    }

    position_type& unit_z()
    {
        return units_[2];
    }

    boost::array<position_type, 3> const& units() const
    {
        return units_;
    }

    boost::array<position_type, 3>& units()
    {
        return units_;
    }

    length_type const Lx() const
    { 
        return 2 * half_extent_[0];
    }

    length_type Lx()
    {
        return 2 * half_extent_[0];
    }

    length_type const Ly() const
    {
        return 2 * half_extent_[1];
    }

    length_type Ly()
    {
        return 2 * half_extent_[1];
    }

    boost::array<length_type, 2> const& half_extent() const
    {
        return half_extent_;
    }

    boost::array<length_type, 2>& half_extent()
    {
        return half_extent_;
    }

    bool operator==(const Plane& rhs) const
    {
        return position_ == rhs.position_ && units_ == rhs.units_ &&
               half_extent_ == rhs.half_extent_;
    }

    bool operator!=(const Plane& rhs) const
    {
        return !operator==(rhs);
    }

    std::string show(int precision)
    {
        std::ostringstream strm;
        strm.precision(precision);
        strm << *this;
        return strm.str();
    }   

protected:
    position_type position_;
    boost::array<position_type, 3> units_;
    boost::array<length_type, 2> half_extent_;
};


template<typename T_>
inline std::pair<typename Plane<T_>::position_type, bool>
deflect(Plane<T_> const& obj, typename Plane<T_>::position_type const& r0, typename Plane<T_>::position_type const& d)
// This routine deflects a displacement on a plane.
// If the displacement vector intersects with the plane (starting from r0) the part
// ranging out of the plane will be deflected into the plane, always into the direction
// towards the plane center.
// ATTENTION! As for now, this only works for displacement vectors perpendicular to the plane
{
  
   // Type abbreviations
   typedef typename Plane<T_>::length_type length_type;
   typedef typename Plane<T_>::position_type position_type;
      
   // Plane properties to make use of
   position_type center_pt = obj.position();
   position_type u_x = obj.unit_x();
   position_type u_y = obj.unit_y();
   position_type u_z = obj.unit_z();   
   
   // Variables used for calculation
   position_type intersect_pt;          // the point at which the displacement vector intersects the edge
   position_type d_out, d_edge, d_perp; // the part of the displacement ranging out of the plane and
                                        // the components parallel and perpendicular to the edge
   position_type d_edge_n;              // normalized d_edge                                        
   position_type icv;                   // = intersect_to_center_vector
   position_type icv_edge, icv_perp;    // the components of icv parallel and perpendicular to the edge
   
   position_type new_pos;
   
   length_type intersect_parameter;
   length_type l_edge, l_perp;
   
   bool changeflag = false;
   
   // Calculate the intersection parameter and intersection point.
   // r0 is the origin position, d is a displacement vector.
   // If intersect_parameter <= 1 we have an intersection.
   if(dot_product(d, u_z) != 0.0)
      intersect_parameter = divide( dot_product(subtract(center_pt, r0), u_z), dot_product(d, u_z) );
   else
      intersect_parameter = (length_type)INT_MAX;       // infinity, displacement is parallel to edge
      
   // Check whether the displacement actually crosses the plane;
   // If not, just return original position plus displacement;
   // if yes, calculate the deflection.
   if(intersect_parameter > 1.0 || intersect_parameter < 0.0){
     
        // No intersection; the new position is just r0+displacement
        new_pos = add(r0, d);
        changeflag = false;
   }
   else{
    
        // Calculate the intersection point and the part of the displacement
        // that ranges out of the target plane.
        // Project all points that are supposed to be in the plane into it,
        // just to be sure.
        intersect_pt = projected_point( obj, add(r0, multiply(d, intersect_parameter)) ).first;
        d_out = multiply(d, subtract(1.0, intersect_parameter));
        
        // Calculate the length of the component of d_out perpendicular to the edge
        // and the vector of the component parallel to the edge
        l_perp = dot_product(d_out, u_z);       // note that this can be positive or negative,
                                                // depending on whether u_z points in or out!
        d_edge = subtract(d_out, multiply(u_z, l_perp));
        d_edge_n = normalize(d_edge);
        
        // Find the vector pointing from the edge towards the center of the plane, which is
        // the component of (center_pt - intersect_pt) perpendicular to the edge.
        // First we calculate the component parallel to the edge
        icv = subtract(center_pt, intersect_pt);
        icv_edge = multiply(d_edge_n, dot_product(icv, d_edge_n));
        icv_perp = subtract(icv, icv_edge);
        
        // Calculate the component perpendicular to the edge in the plane.
        // Note that l_perp can be pos. or neg. while icv_perp always points towards
        // the center of the plane; therefore use abs(l_perp).
        d_perp = multiply( normalize(icv_perp), abs(l_perp) );
        
        // Construct the new position vector, make sure it's in the plane to
        // avoid trouble with periodic boundary conditions
        new_pos = projected_point(obj, add(intersect_pt, add(d_edge, d_perp)) ).first;        
        changeflag = true;
   }
   
   
   // for now this returns the new position without changes
   return std::make_pair( new_pos, changeflag );
}
template<typename T_>
inline bool
is_alongside(Plane<T_> const& obj, typename Plane<T_>::position_type const& pos)
// The function checks if the projection of the position 'pos' is 'inside' the object.
{
    typedef typename Plane<T_>::position_type position_type;

    boost::array<typename Plane<T_>::length_type, 2> half_extends(obj.half_extend());
    position_type pos_vector(subtract(pos, obj.position()));

    return ((abs(dot_product(pos_vector, obj.unit_x())) < half_extends[0]) &&
            (abs(dot_product(pos_vector, obj.unit_y())) < half_extends[1]));
}

template<typename T_>
inline boost::array<typename Plane<T_>::length_type, 3>
to_internal(Plane<T_> const& obj, typename Plane<T_>::position_type const& pos)
// The function calculates the coefficients to express 'pos' into the base of the plane 'obj'
{
    typedef typename Plane<T_>::position_type position_type;
    position_type pos_vector(subtract(pos, obj.position()));

    return array_gen<typename Plane<T_>::length_type>(
        dot_product(pos_vector, obj.unit_x()),
        dot_product(pos_vector, obj.unit_y()),
        dot_product(pos_vector, obj.unit_z()));
}

template<typename T_>
inline std::pair<typename Plane<T_>::position_type,
                 typename Plane<T_>::length_type>
projected_point(Plane<T_> const& obj, typename Plane<T_>::position_type const& pos)
// Calculates the projection of 'pos' onto the plane 'obj' and also returns the coefficient
// for the normal component (z) of 'pos' in the basis of the plane
{
    boost::array<typename Plane<T_>::length_type, 3> x_y_z(to_internal(obj, pos));
    return std::make_pair(
        add(add(obj.position(), multiply(obj.unit_x(), x_y_z[0])),
                                multiply(obj.unit_y(), x_y_z[1])),
        x_y_z[2]);
}

template<typename T_>
inline std::pair<typename Plane<T_>::position_type,
                 typename Plane<T_>::length_type>
projected_point_on_surface(Plane<T_> const& obj, typename Plane<T_>::position_type const& pos)
// Since the projected point on the plane, is already on the surface of the plane,
// this function is just a wrapper of projected point.
{
    return projected_point(obj, pos);
}

template<typename T_>
inline typename Plane<T_>::length_type
distance(Plane<T_> const& obj, typename Plane<T_>::position_type const& pos)
// Calculates the distance from 'pos' to plane 'obj' Note that when the plane is finite,
// and also calculates the distance to the edge of the plane if necessary
{
    typedef typename Plane<T_>::length_type length_type;
    boost::array<length_type, 3> const x_y_z(to_internal(obj, pos));

    length_type const dx(subtract( abs(x_y_z[0]), obj.half_extent()[0]));
    length_type const dy(subtract( abs(x_y_z[1]), obj.half_extent()[1]));

    if (dx < 0 && dy < 0) {
        // pos is positioned over the plane (projected point is in the plane and
	    // not next to it).
        return abs(x_y_z[2]);
    }

    if (dx > 0) // outside the plane in the x direction
    {
        if (dy > 0)
        {
            // outside the plane in both x and y direction
            return std::sqrt(gsl_pow_2(dx) + gsl_pow_2(dy) +
                             gsl_pow_2(x_y_z[2]));
        }
        else
        {
	    // outside the plane in x, but inside in y direction
            return std::sqrt(gsl_pow_2(dx) + gsl_pow_2(x_y_z[2]));
        }
    }
    else   // inside the plane in x direction
    {
        if (dy > 0)
        {
	    // outside the plane in y, but inside in x direction
            return std::sqrt(gsl_pow_2(dy) + gsl_pow_2(x_y_z[2]));
        }
        else
        {
            // inside the plane in both x and y direction (see above)
            return abs(x_y_z[2]);
        }
    }
}

template<typename T, typename Trng>
inline typename Plane<T>::position_type
random_position(Plane<T> const& shape, Trng& rng)
{
    typedef typename Plane<T>::length_type length_type;

    // -1 < rng() < 1. See for example PlanarSurface.hpp.
    return add(
        shape.position(),
        add(multiply(shape.units()[0], shape.half_extent()[0] * rng()),
            multiply(shape.units()[1], shape.half_extent()[1] * rng())));
}

template<typename T>
inline Plane<T> const& shape(Plane<T> const& shape)
{
    return shape;
}

template<typename T>
inline Plane<T>& shape(Plane<T>& shape)
{
    return shape;
}

template<typename T_>
struct is_shape<Plane<T_> >: public boost::mpl::true_ {};

template<typename T_>
struct shape_position_type<Plane<T_> >
{
    typedef typename Plane<T_>::position_type type;
};

template<typename Tstrm_, typename Ttraits_, typename T_>
inline std::basic_ostream<Tstrm_, Ttraits_>& operator<<(std::basic_ostream<Tstrm_, Ttraits_>& strm,
        const Plane<T_>& v)
{
    strm << "{" << v.position() <<  ", " << v.unit_x() << ", " << v.unit_y() << "," << v.Lx() << ", " << v.Ly() << "}";
    return strm;
}


#if defined(HAVE_TR1_FUNCTIONAL)
namespace std { namespace tr1 {
#elif defined(HAVE_STD_HASH)
namespace std {
#elif defined(HAVE_BOOST_FUNCTIONAL_HASH_HPP)
namespace boost {
#endif

template<typename T_>
struct hash<Plane<T_> >
{
    typedef Plane<T_> argument_type;

    std::size_t operator()(argument_type const& val)
    {
        return hash<typename argument_type::position_type>()(val.position()) ^
            hash<typename argument_type::position_type>()(val.unit_x()) ^
            hash<typename argument_type::position_type>()(val.unit_y()) ^
            hash<typename argument_type::length_type>()(val.half_extent()[0]) ^
            hash<typename argument_type::length_type>()(val.half_extent()[1]);
    }
};

#if defined(HAVE_TR1_FUNCTIONAL)
} } // namespace std::tr1
#elif defined(HAVE_STD_HASH)
} // namespace std
#elif defined(HAVE_BOOST_FUNCTIONAL_HASH_HPP)
} // namespace boost
#endif

#endif /* PLANE_HPP */

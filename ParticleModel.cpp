#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <algorithm>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/lexical_cast.hpp>

#include "utils/fun_wrappers.hpp"

#include "ParticleModel.hpp"

// Constructor
ParticleModel::ParticleModel()
{
    // TODO add default structure_type for the bulk?
}

ParticleModel::~ParticleModel()
{
}

// Add a structure type to the model
void ParticleModel::add_structure_type(boost::shared_ptr<structure_type_type> const& structure_type)
{
    // std::pair<structure_type_map_type::iterator, bool> r(
    //     structure_type_map_.insert(std::make_pair(structure->id(), structure)));
    // if (!r.second)
    // {
    //     throw already_exists(
    //         (boost::format("structure id \"%s\" is already used by %s") %
    //             structure->id() %
    //             boost::lexical_cast<std::string>(*(*(r.first)).second)).str());
    // }
    structure_type->bind_to_model(this, species_type_id_generator_());
    structure_type_map_.insert(std::make_pair(structure_type->id(), structure_type));
}

// Get a structure type from the model
boost::shared_ptr<ParticleModel::structure_type_type> ParticleModel::get_structure_type_by_id(structure_type_id_type const& id) const
{
    structure_type_map_type::const_iterator i(structure_type_map_.find(id));
    if (structure_type_map_.end() == i)
    {
        throw not_found(std::string("Unknown structure_type (id=") + boost::lexical_cast<std::string>(id) + ")");
    }

    return (*i).second;
}

// Get all the structure types that are present in the particle model
ParticleModel::structure_type_range ParticleModel::get_structure_types() const
{
    return structure_type_range(
        structure_type_iterator(structure_type_map_.begin(), structure_second_selector_type()),
        structure_type_iterator(structure_type_map_.end(), structure_second_selector_type()));
}

// Get default structure_type. TODO: the default structure_type still has to made by hand
void ParticleModel::set_def_structure_type(boost::shared_ptr<structure_type_id_type> const& id)
{
    // check that the id is actually used.
    structure_type_map_type::const_iterator i(structure_type_map_.find(id));
    if (structure_type_map_.end() == i)
    {
        throw not_found(std::string("Unknown structure_type (id=") + boost::lexical_cast<std::string>(id) + ")");
    }
    default_structure_type_ = id;
}

boost::shared_ptr<ParticleModel::structure_type_id_type> ParticleModel::get_def_structure_type() const
{
    return default_structure_type_;
}


/*
 
 transfer_function.cc - This file is part of MUSIC -
 a code to generate multi-scale initial conditions 
 for cosmological simulations 
 
 Copyright (C) 2010  Oliver Hahn
 
*/


#include "transfer_function.hh"


std::map< std::string, transfer_function_plugin_creator *>& 
get_transfer_function_plugin_map()
{
	static std::map< std::string, transfer_function_plugin_creator* > transfer_function_plugin_map;
	return transfer_function_plugin_map;
}

void print_transfer_function_plugins()
{
	std::map< std::string, transfer_function_plugin_creator *>& m = get_transfer_function_plugin_map();
	std::map< std::string, transfer_function_plugin_creator *>::iterator it;
	it = m.begin();
	std::cout << " - Available transfer function plug-ins:\n";
	while( it!=m.end() )
	{
		if( (*it).second )
			std::cout << "\t\'" << (*it).first << "\'\n";
		++it;
	}	
}

std::unique_ptr<transfer_function_plugin> select_transfer_function_plugin( config_file& cf, const cosmology::parameters& cp )
{
	std::string tfname = cf.get_value<std::string>( "cosmology", "transfer" );
	
	transfer_function_plugin_creator *the_transfer_function_plugin_creator = get_transfer_function_plugin_map()[ tfname ];
	
	if( !the_transfer_function_plugin_creator )
	{	
		std::cerr << " - Error: transfer function plug-in \'" << tfname << "\' not found." << std::endl;
		music::elog.Print("Invalid/Unregistered transfer function plug-in encountered : %s",tfname.c_str() );
		print_transfer_function_plugins();
		throw std::runtime_error("Unknown transfer function plug-in");
		
	}else
	{	
		std::cout << " - Selecting transfer function plug-in \'" << tfname << "\'..." << std::endl;
		music::ulog.Print("Selecting transfer function plug-in  : %s",tfname.c_str() );
	}
	
	return the_transfer_function_plugin_creator->create( cf, cp );
}


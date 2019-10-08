#include "meta.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <dlfcn.h>
#include <unistd.h>
#include <fstream>

using namespace boost::filesystem;

std::vector<std::string> lib_list; // contains full paths to libraries
std::unordered_map<std::string, void*> func_map; // hash table of "function name - function pointer"
std::vector<void*> lib_handle_list;

bool noLibrariesNeeded = false;

void setLibraryList(){
	int found_libs = 0;

	std::string lib_path;
	std::cout << "Enter the full path to the libraries folder: ";
	std::getline(std::cin, lib_path);

	if(lib_path.empty()){
		noLibrariesNeeded = true;
		return;
	}

	path p(lib_path);
	try{
		if( is_directory(p) ){
			for(directory_entry& x : directory_iterator(p) ){
				if( x.path().filename().string().find(".so") != std::string::npos ){
					lib_list.push_back( x.path().string() );
					found_libs++;
				}
			}
		}
	}
	catch(const filesystem_error& exc){
		std::cout << exc.what() << std::endl;
	}

	if(found_libs == 0){
		std::cout << "There was no library found" << std::endl;
		exit(EXIT_FAILURE);
	}
	else{
		std::cout << std::endl << "Found " << found_libs;
		if(found_libs == 1)
			std::cout << " library: ";
		else
			std::cout << " libraries: ";

		std::cout << std::endl;
	}
	for(auto& file_path : lib_list)
		std::cout << "\t" << file_path << std::endl;

	std::cout << std::endl;
}


/* This function imports all the functions from library with LIB_PREFIX prefix(currently "imp"): i.e impFunc, impAvg, etc...
   It is not necessary to do. But if user doesn't want to print every function he defined in library, he has to. 
 */

void importFunction(const std::string& name, void* lib_handle){
	if(name.substr(0, LIB_PREFIX_LENGTH) == LIB_PREFIX){
#ifndef NDEBUG
		std::cout << "Loading symbol " << name << std::endl;
#endif
		void* sym_handle = dlsym(lib_handle, name.c_str());
		if(!sym_handle){
			std::cout << "Error while loading symbol" << name << std::endl;
			exit(EXIT_FAILURE);
		}

		func_map.insert( {
				{ name.substr(LIB_PREFIX_LENGTH) }, // deleting LIB_PREFIX
				{ sym_handle }
			});
	}
#ifndef NDEBUG
	else
		std::cout << "Ignoring symbol " << name << std::endl;
#endif
}


void importLibraries(){ // this function uses "nm" to load list of functions in shared objects
	setLibraryList();

	if(noLibrariesNeeded)
		return;

	for(auto it=lib_list.begin(); it != lib_list.end(); it++)
	{
		lib_handle_list.push_back( // load library
			dlopen((*it).c_str(), RTLD_NOW) ); // or RTLD_LAZY?

		if(!lib_handle_list.back()){ // check if last loaded library was really loaded
			std::cout << "Error while loading library at " << *it << std::endl;
			exit(EXIT_FAILURE);
		}
	}

	/* Dirty way to load functions from library without user typing functions names:
	   1. Call nm -D and parse output file to retrieve symbol names
	   2. Try to load every one of them - if successfully, make table of "function name - function address", we will need it later
	   when parsing functions
	 */

	std::string nm_string = std::string("nm -D ");
	std::string pipe = std::string(" > /tmp/func_names.txt");

	int handle_number = 0;
	for(auto lib_filepath_it = lib_list.begin(); lib_filepath_it != lib_list.end(); lib_filepath_it++, handle_number++){
		std::string command = nm_string + *lib_filepath_it + pipe;
		system(command.c_str());

		std::ifstream handle_sym_names = std::ifstream("/tmp/func_names.txt");
		if(!handle_sym_names.is_open()){
			std::cout << "Error while opening /tmp/func_names.txt..." << std::endl;
			system("rm -f /tmp/func_names.txt");
			exit(EXIT_FAILURE);
		}

		char line[100];
		while(!handle_sym_names.eof()){
			handle_sym_names.getline(line, 100);

			std::string str_line = std::string(line);

			if(str_line.empty())
				break;

			std::string name = str_line.substr(str_line.find_last_of(' ') + 1);

			importFunction(name, lib_handle_list.at(handle_number));
		}
		
		handle_sym_names.close();
	}
	system("rm -f /tmp/func_names.txt");
}

void closeLibraries(){
	if(noLibrariesNeeded)
		return;

	int lib_number = 0;
	for(auto it : lib_handle_list){
		if(dlclose(it)){
			std::string lib_filename = path(lib_list.at(lib_number).begin(), lib_list.at(lib_number).end()).filename().string();
			std::cout << "Unable to close library " << lib_filename << std::endl;
		}
		lib_number++;
	}
}

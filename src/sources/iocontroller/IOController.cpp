/*
 * Copyright (c) 2014, webvariants GmbH, http://www.webvariants.de
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 * 
 * @author: Thomas Krause (thomas.krause@webvariants.de)
 */

#include "iocontroller/IOController.h"

//Constructor
Susi::IOController::IOController() {
	// use current as base path
	this->base_path = Poco::Path(Poco::Path::current()); 
}

Susi::IOController::IOController(std::string base_path) {
	// init with base path
	this->base_path = Poco::Path(base_path);
}

 //high level 
std::size_t Susi::IOController::writeFile(std::string filename,char* ptr, std::size_t len) {

	Poco::Path dir = this->getAbsDirFromString(filename);
	
	if(this->checkDir(dir)) {
		// write File
		std::ofstream s((char*)filename.c_str());
		s<<ptr;
		s.close();

		return len;
	} else {
		throw std::runtime_error{"WriteFile: Dir don't exists!"+filename};
	}
}

std::size_t Susi::IOController::writeFile(std::string filename,std::string content) {
	return this->writeFile(filename, (char*)content.c_str(), content.size());
}

std::string Susi::IOController::readFile(std::string filename) {

	if(this->checkFile(this->getAbsPathFromString(filename))) {
		std::string result = "";
		char * buffer;
		int length = 0;
		std::ifstream s((char*)filename.c_str());
	
		if(s) {
			// get length of file:
	    	s.seekg (0, s.end);
			length = s.tellg();
		    s.seekg (0, s.beg);

	    	buffer = new char [length];
		}

		// read data as a block:
	    s.read(buffer,length); 

	    if (!s) std::cout << "error: only " << s.gcount() << " could be read";
	    
	    s.close();

	    result += std::string(buffer,length);

	    delete[] buffer;

	    return result;
	} else {
		throw std::runtime_error{"ReadFile: File don't exists!"};
	}
}

bool Susi::IOController::movePath(std::string source_path, std::string dest_path) {
	Poco::File sf(this->getAbsPathFromString(source_path));
	Poco::Path dp = this->getAbsPathFromString(dest_path);

	if(sf.exists()) {
		sf.moveTo(dp.toString());
		return true;
	} else {
		throw std::runtime_error{"movePath: SOURCE_PATH don't exist!"};	
	}
}

bool Susi::IOController::copyPath(std::string source_path, std::string dest_path) {
	Poco::File sf(this->getAbsPathFromString(source_path));
	Poco::Path dp = this->getAbsPathFromString(dest_path);

	if(sf.exists()) {
		sf.copyTo(dp.toString());
		return true;
	} else {
		throw std::runtime_error{"copyPath: SOURCE_PATH don't exist!"};	
	}
}

bool Susi::IOController::deletePath(std::string path) {

	Poco::File f(this->getAbsPathFromString(path));

	if(f.exists()) {
		if(f.isFile() || f.isDirectory()) {
			if(f.isFile()) {
				f.remove(false);
				return true;
			} else {
				f.remove(true); // deletes recursive
				return true;
			}
		} else {
			throw std::runtime_error{"Delete: PATH is no FILE or DIR! (maybe PATH is LINK or DEVICE)"};	
		}
	} else {
		return false;
	}
}

// low level
bool Susi::IOController::makeDir(std::string dir) {

	Poco::Path p(true);

	//std::cout<<p.isAbsolute()<<std::endl;

	p.append(this->base_path);
	p.append(dir);

	/*
	if(!p.isAbsolute()) {
		p.makeAbsolute(this->base_path);
		std::cout<<"APPENDED:"<<p.toString()<<std::endl;
	} else {
		std::cout<<"ABSOLUT:"<<p.toString()<<std::endl;
	}
*/
	//std::cout<<"APPENDED:"<<p.toString()<<std::endl;
	//std::cout<<"FILENAME:"<<p.getFileName()<<std::endl;

	Poco::File f(p);
	f.createDirectories();

	/*
	Poco::Path p(Poco::Path::current());
	Poco::File f;

	p.pushDirectory("Thomas");
	p.pushDirectory("Test");

	//p.setFileName("test.dat");

	f = Poco::File(p);
	f.createDirectories();

	std::string s = p.toString(); 

	std::cout<<s<<" "<<p.depth()<<std::endl;

	Poco::Path p2("./");

	std::string s2 = p2.toString(); 

	std::cout<<s2<<" "<<p2.depth()<<std::endl;

	std::cout 
		<< "cwd: "  << Poco::Path::current() << std::endl
		<< "home: " << Poco::Path::home() << std::endl
		<< "temp: " << Poco::Path::temp() << std::endl
		<< "null: " << Poco::Path::null() << std::endl;
*/
	return true;
}

bool Susi::IOController::setExecutable(std::string path, bool isExecutable) {
	Poco::File f(this->getAbsPathFromString(path));

	if(f.exists()) {
		f.setExecutable(isExecutable);
		return true;
	} else {
		throw std::runtime_error{"setExecutable: PATH is no FILE or DIR!"};
	}
}

bool Susi::IOController::getExecutable(std::string path) {
	Poco::File f(this->getAbsPathFromString(path));

	if(f.exists()) {
		return f.canExecute();
	} else {
		throw std::runtime_error{"getExecutable: PATH is no FILE or DIR!"};
	}
}

// helper function
Poco::Path Susi::IOController::getAbsPathFromString(std::string path) {
	Poco::Path p(path);
	if(p.isRelative()) 
		p.makeAbsolute(this->base_path);
	return p;	
}

Poco::Path Susi::IOController::getAbsDirFromString(std::string path) {
	Poco::Path p = this->getAbsPathFromString(path);
	p.makeParent();
	return p;
}

bool Susi::IOController::checkDir(Poco::Path dir) {
	
	Poco::File f(dir);

	if(f.exists()) {
		if(f.isDirectory()) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

bool Susi::IOController::checkFile(Poco::Path dir) {
	Poco::File f(dir);

	if(f.exists()) {
		if(f.isFile()) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}	
}
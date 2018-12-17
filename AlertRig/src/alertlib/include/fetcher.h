/*
 * fetcher.h
 *
 *  Created on: Nov 6, 2018
 *      Author: dan
 */

#ifndef FETCHER_H_
#define FETCHER_H_

#include <map>
#include <boost/filesystem.hpp>
#include <string>

namespace fs = boost::filesystem;


// Abstract base class
// Create a subclass of this, instantiate it, and pass it to the FetcherWithFunctor constructor.
// The FWF calls the () operator with each file encountered in the base folder.
// The()(const path& p, K& key) operator should
//
// (1) be defined in your code with the actual type used as your key (i.e. the template parameter)....
//     for example if you want to use a string as the key, then your subclass should look like this:
//     class MyPathMatcher: public PathMatcher<string>
//     {
//		public:
//			MyPathMatcher(/* any args you want here - this is key advantage over the func pointer case */) {};
//			bool operator()(const path& p, string& key)
//          { /* return true if file in path 'p' should go into the map, false otherwise */
//            /* If returning true, also set the value of the key - it will be used to store 'p' in the map */
//
template<class K>
class PathMatcher {
public:
  PathMatcher() {};
  virtual ~PathMatcher() {};

  // This method takes a file path (full file path, but within the base path)
  // and should return true if the file should be included in the map (Fetcher).
  // If returning true, then also set the key value. The file will be fetchable with
  // fetcher.at(key)
  virtual bool operator() (const fs::path& p, K& key) = 0;
};


template<class K>
class FetcherWithFunctor: public std::map<K, boost::filesystem::path>
{
	PathMatcher<K>& m_pathMatcher;
	fs::path m_base;
	void scan_path(const fs::path& p)
	{
		K key;
		if (is_regular_file(p) && m_pathMatcher(p, key))
		{
			this->insert(make_pair(key, p));
			//cerr << "scan_path - got file " << p.string() << " - key " << key << endl;
		}
		else if (is_directory(p))
		{
			//cerr << "scan_path - got dir " << p.string() << endl;
			for (auto&& x : boost::filesystem::directory_iterator(p))
				scan_path(x.path());
		}

#if 0
		fs::path basepath("/home");
		fs::path newpath("/home/apple/one");
		fs::path diffpath;

		fs::path tmppath = newpath;
		while (tmppath != basepath) {
			diffpath = tmppath.stem() / diffpath;
			tmppath = tmppath.parent_path();
		}
#endif
	}


	// non-recursive version
	void scan(const fs::path& p)
	{
		K key;
		for (fs::recursive_directory_iterator end, dir(p); dir != end; ++dir)
		{
			if (is_regular_file(*dir))
			{
				fs::path diffpath;
				fs::path temppath = *dir;
				while (temppath != p)
				{
					diffpath = temppath.filename() / diffpath;
					temppath = temppath.parent_path();
				}
				cout << "diff: " << diffpath.string() << std::endl;
				if (m_pathMatcher(diffpath, key))
				{
					this->insert(make_pair(diffpath.string(), *dir));
				}
			}
		}
	}



public:
	FetcherWithFunctor(PathMatcher<K>& pm)
		: m_pathMatcher(pm)
	{}

	FetcherWithFunctor(const std::string& folder, PathMatcher<K>& pm)
		: m_pathMatcher(pm)
	{
		boost::filesystem::path p(folder);
		//scan_path(p);
	}

	void addFolder(const std::string& folder)
	{
		boost::filesystem::path p(folder);
		scan(p);
		//scan_path(p);
	}
};



#endif /* FETCHER_H_ */

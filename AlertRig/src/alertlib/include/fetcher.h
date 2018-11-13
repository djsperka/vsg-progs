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
  virtual bool operator() (const boost::filesystem::path& p, K& key) = 0;
};


template<class K>
class FetcherWithFunctor: public std::map<K, boost::filesystem::path>
{
	PathMatcher<K>& m_pathMatcher;

	void scan_path(const boost::filesystem::path& p)
	{
		K key;
		//cerr << "scan_path " << p.string() << endl;
		if (is_regular_file(p) && m_pathMatcher(p, key))
			this->insert(make_pair(key, p));
		else if (is_directory(p))
		{
			for (auto&& x : boost::filesystem::directory_iterator(p))
				scan_path(x.path());
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
		scan_path(p);
	}

	void addFolder(const std::string& folder)
	{
		boost::filesystem::path p(folder);
		scan_path(p);
	}
};



#endif /* FETCHER_H_ */

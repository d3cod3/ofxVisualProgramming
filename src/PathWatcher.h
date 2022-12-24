/*==============================================================================

	PathWatcher.h

	Copyright (C) 2016 Dan Wilcox <danomatika@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program. If not, see <http://www.gnu.org/licenses/>.

	Adapted from https://github.com/NickHardeman/ofxFileWatcher

==============================================================================*/
#pragma once

#include <string>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <sys/stat.h>

/// \class PathWatcher
/// \brief watch file and directory paths for modifications
///
/// detects creation, modification, and deletion/move events
///
/// can be used with a callback or via an event queue
///
/// Example queue/poll usage:
///
///     PathWatcher watcher;
///
///     ...
///
///     // poll for events
///     watcher.update();
///
///     // check for any waiting events
///     while(watcher.waitingEvents()) {
///
///         // get next event in the queue
///         PathWatcher::Event event = watcher.nextEvent();
///
///         /// process the event
///         switch(event.change) {
///             case PathWatcher::CREATED:
///                 std::cout << "path created: " << event.path << std::endl;
///                 break;
///             case PathWatcher::MODIFIED:
///                 std::cout << "path modified: " << event.path << std::endl;
///                 break;
///             case PathWatcher::DELETED:
///                 std::cout << "path deleted: " << event.path << std::endl;
///                 break;
///             default: // NONE
///                 break;
///         }
///      }
///
/// Example queue/thread usage:
///
///     PathWatcher watcher;
///
///     // start the thread, otherwise call update() to check manually
///    	watcher.start();
///
///     ...
///
///     // check for any waiting events
///     while(watcher.waitingEvents()) {
///
///         // get next event in the queue
///         PathWatcher::Event event = watcher.nextEvent();
///
///         /// process the event
///         switch(event.change) {
///             case PathWatcher::CREATED:
///                 std::cout << "path created: " << event.path << std::endl;
///                 break;
///             case PathWatcher::MODIFIED:
///                 std::cout << "path modified: " << event.path << std::endl;
///                 break;
///             case PathWatcher::DELETED:
///                 std::cout << "path deleted: " << event.path << std::endl;
///                 break;
///             default: // NONE
///                 break;
///         }
///     }
///
/// Example callback/thread usage:
///
///     PathWatcher watcher;
///
///     // add a path to watch
///	    watcher.addPath("test.txt");
///
///     // start the thread
///    	watcher.start();
///
///     // set callback as a function pointer or lambda
///     watcher.setCallback([](const PathWatcher::Event &event) {
///
///         // this is called within the watcher's thread, so you will need
///         // to protect any shared resources with a mutex or atomics
///
///         switch(event.change) {
///             case PathWatcher::CREATED:
///                 std::cout << "path created: " << event.path << std::endl;
///                 break;
///             case PathWatcher::MODIFIED:
///                 std::cout << "path modified: " << event.path << std::endl;
///                 break;
///             case PathWatcher::DELETED:
///                 std::cout << "path deleted: " << event.path << std::endl;
///                 break;
///             default: // NONE
///                 break;
///         }
///     });
///
class PathWatcher {

	public:

		PathWatcher() {
			running = false;
			removeDeleted = false;
		}
		virtual ~PathWatcher() {stop();}

	/// \section Paths

		/// add a path to watch, full or relative to current directory
		/// optionally set contextual name
		void addPath(const std::string &path, const std::string &name="") {
			mutex.lock();
			std::vector<Path>::iterator iter = std::find_if(paths.begin(), paths.end(),
				[&path](Path const &p) {
					return p.path == path;
				}
			);
			if(iter == paths.end()) {
				paths.push_back(Path(path, name));
			}
			mutex.unlock();
		}

		/// remove a watched path
		void removePath(const std::string &path) {
			mutex.lock();
			std::vector<Path>::iterator iter = std::find_if(paths.begin(), paths.end(),
				[&path](Path const &p) {
					return p.path == path;
				}
			);
			if(iter != paths.end()) {
				paths.erase(iter);
			}
			mutex.unlock();
		}

		/// remove a watched path by name
		void removePathByName(const std::string &name) {
			mutex.lock();
			std::vector<Path>::iterator iter = std::find_if(paths.begin(), paths.end(),
				[&name](Path const &p) {
					return p.name == name;
				}
			);
			if(iter != paths.end()) {
				paths.erase(iter);
			}
			mutex.unlock();
		}

		/// remove all watched paths
		void removeAllPaths() {
			mutex.lock();
			paths.clear();
			mutex.unlock();
		}

		/// does a path exist?
		static bool pathExists(const std::string & path) {
			#ifdef TARGET_WIN32
				return _access(path.c_str(), 0) == 0;
			#else
				return access(path.c_str(), F_OK) == 0;
            #endif

		}

	/// \section Watching for Changes

		/// the type of change
		enum ChangeType {
			NONE,     //< path has not changed
			CREATED,  //< path was created
			MODIFIED, //< path was modified
			DELETED   //< path was deleted or moved
		};

		/// a change event
		struct Event {
			ChangeType change = NONE; //< no change, created, modified, deleted
			std::string path;  //< path, relative or absolute
			std::string name;  //< optional contextual name
		};

		/// manually check for changes, returns true if a change was detected
		/// pushes change onto the queue or calls callback function if set
		bool update() {
			bool changed = false;
			mutex.lock();
			auto iter = paths.begin();
			while(iter != paths.end()) {
				Path &path = (*iter);
				ChangeType change = path.changed();
				if(change != NONE) {
					changed = true;
					Event event;
					event.change = change;
					event.path = path.path;
					event.name = path.name;
					if(callback) {
						callback(event);
					}
					else {
						queue.push(event);
					}
					if(change == DELETED && removeDeleted) {
						paths.erase(iter);
						continue;
					}
				}
				iter++;
			}
			mutex.unlock();
			return changed;
		}

		/// remove deleted paths automatically? (default: false)
		void setRemoveDeletedPaths(bool remove) {
			removeDeleted = remove;
		}

		/// manually remove any deleted or non-existing paths
		void removeDeletedPaths() {
			auto iter = paths.begin();
			while(iter != paths.end()) {
				Path &path = (*iter);
				if(!path.exists) {
					paths.erase(iter);
				}
				iter++;
			}
		}

	/// \section Event Queue

		/// returns true if there are any waiting events
		bool waitingEvents() {
			return !queue.empty();
		}

		/// get the next event in the queue
		/// returns an event with ChangeType of NONE if the queue is empty
		Event nextEvent() {
			std::lock_guard<std::mutex> lock(mutex);
			if(queue.empty()) {
				return Event();
			}
			else {
				Event e = queue.front();
				queue.pop();
				return e;
			}
		}

	/// \section Thread

		/// set optional callback to receive change events
		///
		/// called within the watcher's thread, so you will need
		/// to protect any shared resources with a mutex or atomics
		///
		/// function:
		///
		///     void pathChanged(const PathWatcher::Event &event) {
		///         cout << "path changed: " << event.path << endl;
		///     }
		///
		///     int main() {
		///         PatchWatcher watcher;
		///         watcher.setCallback(pathChanged);
		///         watcher.start();
		///         ...
		///     }
		///
		/// class member function:
		///
		///     class Foo {
		///         public:
		///             void setup();
		///             void pathChanged(const PathWatcher::Event &event);
		///     };
		///     ...
		///     void Foo::setup() {
		///         std::function<void(const PathWatcher::Event &)> cb;
		///
		///         // via lambda
		///         cb = [this](const PathWatcher::Event &event) {
		///             pathChanged(event);
		///         };
		///
		///         // or function pointer
		///         cb = std::bind(&Foo::pathChanged, this, std::placeholders::_1);
		///
		///         PatchWatcher watcher;
		///         watcher.setCallback(cb);
		///         watcher.start();
		///         ...
		///     }
		///
		void setCallback(std::function<void(const PathWatcher::Event &event)> const &callback) {
			mutex.lock();
			this->callback = callback;
			mutex.unlock();
		}

		/// start a background thread to automatically check for changes,
		/// sleep sets how often to check in ms
		void start(unsigned int sleep=500) {
			if(!running) {
				running = true;
				thread = new std::thread([this,sleep]{
					while(running) {
						update();
						std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
					}
				});
			}
		}

		/// stop the background thread
		void stop() {
			if(running) {
				running = false;
				thread->join();
				delete thread;
				thread = nullptr;
			}
		}

		/// is the thread currently running?
		bool isRunning() {return running;}

	protected:

		/// a path to watch
		class Path {

			public:

				std::string path;    //< relative or absolute path
				std::string name;	 //< optional contextual name
				long modified = 0;   //< last modification st_mtime
				bool exists = true;  //< does the path exist?

				/// create a new Path to watch with optional name
				Path(const std::string &path, const std::string &name="") {
					this->path = path;
					this->name = name;
					if(pathExists(path)) {
						update();
					}
					else {
						exists = false;
					}
				}

				/// returns detected change type or NONE
				ChangeType changed() {
					if(pathExists(path)) {
						if(exists) {
							struct stat attributes;
							stat(path.c_str(), &attributes);
							if(modified != attributes.st_mtime) {
								modified = attributes.st_mtime;
								return MODIFIED;
							}
						}
						else {
							update();
							exists = true;
							return CREATED;
						}
					}
					else if(exists) {
						modified = 0;
						exists = false;
						return DELETED;
					}
					return NONE;
				}

				/// update modification time
				void update() {
					struct stat attributes;
					stat(path.c_str(), &attributes);
					modified = attributes.st_mtime;
				}
		};

		std::vector<Path> paths;         //< paths to watch
		std::atomic<bool> removeDeleted; //< remove path when deleted?

		std::queue<Event> queue; //< event queue

		/// change event callback function pointer
		std::function<void(const PathWatcher::Event &event)> callback = nullptr;

		std::atomic<bool> running;     //< is the thread running?
		std::thread *thread = nullptr; //< thread
		std::mutex mutex;              //< thread data mutex
};

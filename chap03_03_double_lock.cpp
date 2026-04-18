void init() {
    if (!resource_ptr) {          // Check 1
        lock_guard<mutex> lk(m);
        if (!resource_ptr) {      // Check 2
            resource_ptr = new Resource();
        }
    }
}

//Meyer's Singleton, gurantee by standard
Resource& get_resource(){
  static Resource resource;
  return resource;
}

#include <iostream>
#include <thread>
#include <mutex> // Required for once_flag and call_once
#include <vector>

class Database {
private:
    // 1. The 'memory' of the initialization. 
    // It must be a member variable (or static) to persist.
    std::once_flag connection_flag;

    void open_connection() {
        // This is the protected "once-only" logic
        std::cout << "[Thread " << std::this_thread::get_id() 
                  << "] Connecting to Database..." << std::endl;
    }

public:
    void init() {
        // 2. The 'call_once' replaces the double-if-check and the manual mutex.
        // If 10 threads hit this, 1 runs it, and 9 wait until it's done.
        std::call_once(connection_flag, &Database::open_connection, this);
    }

    void query() {
        std::cout << "Executing query..." << std::endl;
    }
};

int main() {
    Database db;
    std::vector<std::jthread> workers;

    for (int i = 0; i < 10; ++i) {
        workers.emplace_back([&db]() {
            db.init(); // Every worker tries to init
            db.query();
        });
    }

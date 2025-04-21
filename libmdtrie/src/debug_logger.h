#ifndef DEBUG_LOGGER_H
#define DEBUG_LOGGER_H

#include <fstream>
#include <mutex>
#include <string>
#include <iostream>

class DebugLogger {
public:
    // Returns the singleton instance.
    static DebugLogger& getInstance() {
        static DebugLogger instance;
        return instance;
    }
    
    // Initialize the output file. Call this before logging.
    void init(const std::string &filename) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if(m_out.is_open()){
            m_out.close();
        }
        m_out.open(filename, std::ios::out | std::ios::app);
        if(!m_out.is_open()){
            std::cerr << "Failed to open debug file: " << filename << std::endl;
        }
    }
    
    // Log a message in a thread-safe manner.
    void log(const std::string &msg) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_out.is_open()) {
            m_out << msg << std::endl;
        } else {
            // Optionally, write to stderr if no file is open.
            std::cerr << msg << std::endl;
        }
    }

    // Template member function to log an array with its name.
    template <typename T, std::size_t N>
    void logArray(const std::string &arrayName, const T (&arr)[N]) {
        std::ostringstream oss;
        oss << arrayName << " = [";
        for (std::size_t i = 0; i < N; i++) {
            oss << arr[i];
            if (i < N - 1) {
                oss << ", ";
            }
        }
        oss << "]";
        log(oss.str());
    }
    
    // Delete copy and assignment.
    DebugLogger(const DebugLogger&) = delete;
    DebugLogger& operator=(const DebugLogger&) = delete;

private:
    DebugLogger() {}  // no default file open; must call init()
    ~DebugLogger() { if(m_out.is_open()) m_out.close(); }

    std::ofstream m_out;
    std::mutex m_mutex;
};

// Define the DEBUG_LOG macro here.
#define DEBUG_LOG(msg) DebugLogger::getInstance().log(msg)

#endif // DEBUG_LOGGER_H

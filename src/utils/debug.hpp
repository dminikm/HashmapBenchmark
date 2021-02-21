#pragma once
#include <fstream>
#include <thread>
#include <sstream>

class DebugStream {
    public:
        DebugStream() {
#ifdef DEBUG_LOG
            std::stringstream stream;

            stream  << "debug_thread_"
                    << std::this_thread::get_id()
                    << ".log"; 

            this->log_file = std::ofstream(stream.str());
#endif
        }

        template <typename T>
        auto operator<<(const T& value) -> DebugStream& {
#ifdef DEBUG_LOG
            this->log_file << value;
#endif
            return *this;
        }

        auto operator<<(std::ostream& (*value)(std::ostream&)) -> DebugStream& {
#ifdef DEBUG_LOG
            this->log_file << value;
#endif
            return *this;
        }

    private:
#ifdef DEBUG_LOG
        std::ofstream log_file;
#endif
};

inline auto dbg() -> DebugStream& {
    // Each thread gets it's own stream
    static thread_local DebugStream stream;
    return stream;
}

#ifdef DEBUG_LOG
    #define DBG(x) dbg() x
#else
    #define DBG(x)
#endif
// Defines some macros that I basically need everywhere.

#ifndef ECOSYSTEM_AUTOMATA_MACROS_H_
#define ECOSYSTEM_AUTOMATA_MACROS_H_

// This requires c++ '11.
#define DISSALOW_COPY_AND_ASSIGN(T) \
    T & operator=(const T & other) = delete; \
    T(const T & other) = delete;

#endif

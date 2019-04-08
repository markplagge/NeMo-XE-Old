//
// Created by plaggm on 4/4/19.
//

#ifndef NEMO2_INFO_DISPLAY_H
#define NEMO2_INFO_DISPLAY_H
#include <include/rang.hh>
#include <iostream>
#include <ross.h>
static class NeMessage{
public:
    static void ne_err(size_t l, size_t f, const char * s);
    template <typename T>
    static void ne_err(size_t l, size_t f, const char * s, T o);
    template <typename T, typename ... Args>
    static void ne_err(size_t l, size_t f, const char * s, T o, Args... args);

    static void ne_info(size_t l, size_t f, const char *s);
    template <typename T>
    static void ne_info(size_t l, size_t f, const char *s , T o);
    template <typename T, typename ... Args>
    static void ne_info(size_t l, size_t f, const char *s, Args... args);
};
#endif //NEMO2_INFO_DISPLAY_H

/*
 * util.h
 * Copyright (C) 2013 uqcskenn <uqcskenn@hawke>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef __UTIL_H__
#define __UTIL_H__
#include <string>
#include <algorithm>
template < class ContainerT >
void tokenize(const std::string& str, ContainerT& tokens, const std::string& delimiters = " \t", const bool trimEmpty = false)
{
    std::string::size_type pos, lastPos = 0;
    while(true)
    {
        pos = str.find_first_of(delimiters, lastPos);
        if(pos == std::string::npos)
        {
            pos = str.length();

            if(pos != lastPos || !trimEmpty)
                tokens.push_back( typename ContainerT::value_type(str.data()+lastPos, (typename ContainerT::value_type::size_type)pos - lastPos ));

            break;
        }
        else
        {
            if(pos != lastPos || !trimEmpty)
                tokens.push_back(typename ContainerT::value_type(str.data()+lastPos, (typename ContainerT::value_type::size_type)pos-lastPos ));
        }

        lastPos = pos + 1;
    }
}

#endif /* !__UTIL_H__ */



#include "include/globals.h"
/*

	Copyright © 2015-2019 Mark Plagge.



	The MIT License (MIT)

	Copyright (c) 2018

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.


*/





static int void_function(void) {
    return 0;
}



unsigned long get_neurosynaptic_tick(double now) {
    return (unsigned long) now;
}

unsigned long get_next_neurosynaptic_tick(double now){
    return (unsigned long) now + 1;
}
using namespace std;
template<typename ... Args>
string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}


std::string Nemo_Message::to_string() {
    std::string message_type_desc = "HEARTBEAT";
    if(message_type == NEURON_SPIKE) {
        message_type_desc = "NEURON SPIKE";
    }
    std::string result = string_format("%s,%i,%i,%i,%i,%i,%f\n",message_type_desc.c_str(),source_core,dest_axon,intended_neuro_tick,nemo_event_status,
                                       random_call_count,debug_time);
    return result;
}

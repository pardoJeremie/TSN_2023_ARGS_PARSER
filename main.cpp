//
//  main.cpp
//  TSN_2023_args_parser
//
//  Created by pardo jérémie on 16/11/2023.
//

#include <iostream>
#include "argparse.hpp"

struct MyOptions : public arg::Options
{
    arg::pairValInit<int32_t> opt1 = op<int32_t>("i,opt1", "option 1 description", -1);
    // default value = -1
    
    arg::pairValInit<uint64_t> opt2 = op<uint64_t>("u,opt2", "option 2 description");
    // no default value, default constructed
    
    arg::pairValInit<bool> opt3 = op<bool>("b,opt3", "option 3 description");
    // default false
    
    arg::pairValInit<std::string> opt4 = op<std::string>("s,opt4", "option 4 description","default value");
    // default value = "default value"
};

int main(int argc, const char * argv[]) {
    // insert code here...
    auto options = arg::Options::parse<MyOptions>(argc, argv);

    std::cout << "result: args opt1 = " << (options.opt1.second? std::to_string(options.opt1.first): "?" )
    << "\n        args opt2 = " << (options.opt2.second? std::to_string(options.opt2.first): "?" )
    << "\n        args opt3 = " << (options.opt3.second? std::to_string(options.opt3.first): "?" )
    << "\n        args opt4 = " << (options.opt4.second? options.opt4.first.get(): "?" ) << std::endl;
    
    return 0;
}

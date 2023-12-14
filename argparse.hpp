//
//  argparse.hpp
//  TSN_2023_args_parser
//
//  Created by pardo jérémie on 16/11/2023.
//

#ifndef argparse_hpp
#define argparse_hpp

#include <iostream>
#include <stdio.h>
#include <variant>
#include <functional>
#include <optional>
#include <string>
#include <sstream>
#include <memory>
#include <vector>
#include <regex>
#include <algorithm>
#include <cctype>
#include <type_traits>
#include <tuple>
#include <limits>
#include <iomanip>

namespace arg {

    template<class T>
    using ref = std::reference_wrapper<T>;
    
    template<class T>
    using pairValInit = std::pair<arg::ref<T>,arg::ref<bool>>; // Contain the value and the initialized status of the arg of the option

    using var = std::variant<
        bool,
        int32_t,
        uint32_t,
        int64_t,
        uint64_t,
        std::string>;

    namespace details {
        template<class... Ts> struct overload : Ts ... { using Ts::operator()...; };
        template<class... Ts> overload(Ts...) -> overload<Ts...>;

        /**
         * Permet de mettre à jour "variant" avec "argv_str_value".
         * Convertir la chaine de charactere (argv_str_value) en entier si besoin.
         */
        void convert(const char *const argv_str_value, var& variant) noexcept(false)
        {
            // Utiliser std::visit() et overload{}
            std::visit(overload {
                [&](std::string& a){ a = argv_str_value;},
                [&](auto& a){
                    std::string s = argv_str_value;
                    if(!std::any_of(s.begin(),s.end(),::isdigit))
                        throw std::runtime_error("input value don't contain a digit!");
                    
                    auto b = strtol(argv_str_value,NULL,10);
                    
                    if (std::is_same<bool&,decltype(a)>::value && (b != 1 || b != 0)) // attention, all possible type of the variable 'a' are reference!
                        throw std::runtime_error("input value is not 0 or 1!");
                    else if (std::is_unsigned<decltype(a)>::value && b < 0)
                        throw std::runtime_error("input value is negative!");
                    else if (true) //si la valeur est trop grande !!!!
                        throw std::runtime_error("input value is is to big!");
                
                    a = static_cast< std::remove_reference_t<decltype(a)>>(b); // remove the reference of var&
                    // remark: using the equivalent std::remove_reference<decltype(a)>::type create a "Missing 'typename' ... " error
                }}, variant);
        }

        /**
         * Permet de convertir un variant en std::string.
         */
        std::string to_string(const var& v) noexcept
        {
            // Utiliser std::visit() et overload{}
            std::stringstream ss;
            
            std::visit(overload {
                [&](const bool& a){ss<<(a?"true":"false");},
                [&](const auto& a){ss<<a;}}, v);
            
            return ss.str();
        }
    
        std::string varType_to_string(const var& v) noexcept // typeid(decltype(a)).name() don't return a satifiable name
        {
            std::string s;
        
            std::visit(overload {
                [&](const bool& a){s="bool";},
                [&](const int32_t& a){s="int32_t";},
                [&](const uint32_t& a){s="uint32_t";},
                [&](const int64_t& a){s="int64_t";},
                [&](const uint64_t& a){s="uint64_t";},
                [&](const std::string& a){s="string";}}, v);
        
            return s;
        }
    }

    class Options
    {
    public:

        /**
         * Permet d'ajouter une option à "m_options".
         *
         * IMPORTANT:
         * On retourne une référence sur la valeur du variant stockée dans "m_options".
         *
         * Mettre à jour la valeur du variant avec la valeur par défaut (opt) si besoin.
         *
         * "name" est sous la forme "i,opt1", il faut split la chaine avant de la stocker. 'i' est l'*alias* et "opt1" est l'*id* défini dans *ts_ids*.
         */
        
        
        template<typename T>
        pairValInit<T> op(const std::string& name, const std::string& help, const std::optional<T>& opt = std::nullopt) noexcept(false)
        {
            T val;
            bool as_default_Val = (opt != std::nullopt);
            
            if(as_default_Val)
                val = opt.value();
            else if (std::is_same<bool,T>::value) { // init bool opt value if no value is set;
                val = false;
                as_default_Val = true;
            }
            
            if (!(name.size() > 2 && std::regex_match(name,std::regex("[a-zA-Z],[a-zA-Z0-9]*"))))
                throw std::runtime_error("option have wrong alias and name format! the format should be \"[a-zA-Z],[a-zA-Z0-9]*\" ");
            char alias = name.front();
            std::string option_name = name;
            option_name.erase(0,2);
            
            // Utiliser m_options.emplace_back() qui retourne une référence sur l'objet ajouté.
            auto& opt_ref = m_options.emplace_back(std::make_unique<ts_option>( ts_option{ts_ids{option_name,alias}, val, help, as_default_Val}));

            pairValInit<T> op_val_ref = { std::ref(std::get<T>(opt_ref.get()->value)), std::ref<bool>(opt_ref.get()->initialized)};
            
            return op_val_ref;
        }

        /**
         * Parse les arguments du programme (argv).
         */
        void parse(int argc, const char *const *argv) // noexcept ou noexcept(false)
        {
            // Parcourir "m_options" pour mettre à jour les valeurs en fonction de "argv".
            for(;false;) {
                
            }
        }

        template<typename T>
        static T parse(int argc, const char *const *argv) noexcept {
            T args{};
            args.parse(argc, argv);
            return args;
        }

        private:
        
        /**
         * Affiche les options avec leurs valeurs par defaut.
         */
        void help() noexcept
        {
            std::stringstream ss;
            
            ss << "Usage:\n\t-h,--help\n";
            
            for(std::vector<std::unique_ptr<ts_option>>::iterator it = m_options.begin(); it != m_options.end(); ++it) {
                ss << "\t-" << it->get()->ids.alias << ",--" << it->get()->ids.id << " : " << details::varType_to_string(it->get()->value);
                
                if(it->get()->initialized) // Since the fonction can only be called via '-h,--help', 'initialized == true' implies that the variant have a default value.
                    ss << " (" << details::to_string(it->get()->value) <<")";
                
                ss <<"\n";
            }
            
            std::cout << ss.str() << std::endl;
        }
        
        /**
         * Les differents nom de l'option
         */
        struct ts_ids final {
            std::string id{};   // nom long (ex: "opt1")
            char alias = '\0';  // alias (ex: 'i')
        };

        /**
         * Définition d'une option
         */
        struct ts_option final {
            ts_ids ids{};       // nom de l'option
            var value;          // valeur de l'option
            std::string help{}; // description de l'option
            bool initialized = false; // value as default or is initialized
        };
        
        std::vector<std::unique_ptr<ts_option>> m_options{};

    };

}
#endif /* argparse_hpp */

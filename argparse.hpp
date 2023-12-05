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


namespace arg {

    template<class T>
    using ref = std::reference_wrapper<T>;

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
        bool convert(const char *const argv_str_value, var& variant)
        {
            bool converted = true;
            // Utiliser std::visit() et overload{}
            std::visit(overload {
                [&](bool& a){converted = false;}, // should never be used
                [&](int32_t& a){
                    auto b = strtol(argv_str_value,NULL,10); // remarque If no valid conversion could be performed, a zero value is returned from strtol
                    a = static_cast<int32_t>(b); },
                [&](uint32_t& a){
                    auto b = strtol(argv_str_value,NULL,10);
                    if (b < 0)
                        converted = false;
                    else
                        a = static_cast<uint32_t>(b);},
                [&](int64_t& a){
                    auto b = strtol(argv_str_value,NULL,10);
                    a = static_cast<int32_t>(b); },
                [&](uint64_t& a){
                    auto b = strtol(argv_str_value,NULL,10);
                    if (b < 0)
                        converted = false;
                    else
                        a = static_cast<uint64_t>(b);},
                [&](std::string& a){ a = argv_str_value;}}, variant);

            return converted;
        }

        /**
         * Permet de convertir un variant en std::string.
         */
        std::string to_string(const var& v)
        {
            // Utiliser std::visit() et overload{}
            std::stringstream ss;
            
            std::visit(overload {
                [&](const bool& a){ss<<(a?"true":"false");},
                [&](const int32_t& a){ss<<a;},
                [&](const uint32_t& a){ss<<a;},
                [&](const int64_t& a){ss<<a;},
                [&](const uint64_t& a){ss<<a;},
                [&](const std::string& a){ss << '\"' << a << '\"';}}, v);
            
            return ss.str();
        }
    }

    class Options
    {
    private:
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
        };
        
        std::vector<std::unique_ptr<ts_option>> m_options{};

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
        ref<T> op(const std::string& name, const std::string& help, const std::optional<T>& opt = std::nullopt)
        {
            T val;
            
            if(opt != std::nullopt)
                val = opt.value();
            
            if (!(name.size() > 2 && std::regex_match(name,std::regex("[a-zA-Z],[a-zA-Z0-9]*"))))
                throw; // trow exception since the name is wrong
            char alias = name.front();
            std::string option_name = name;
            option_name.erase(0,1);
            
            // Utiliser m_options.emplace_back() qui retourne une référence sur l'objet ajouté.
            ref<T> op_val_ref = std::ref(std::get<T>(
                m_options.emplace_back(
                    std::make_unique<ts_option>(
                        ts_option{ts_ids{name,alias}, val, help})).get()->value));
            
            return op_val_ref;
        }

        /**
         * Affiche les options avec leurs valeurs par defaut.
         */
        void help()
        {

        }

        /**
         * Parse les arguments du programme (argv).
         */
        void parse(int argc, const char *const *argv)
        {
            // Parcourir "m_options" pour mettre à jour les valeurs en fonction de "argv".
            for(;false;) {
                
            }
        }

        template<typename T>
        static T parse(int argc, const char *const *argv) {
            T args{};
            args.parse(argc, argv);
            return args;
        }
    };
}
#endif /* argparse_hpp */

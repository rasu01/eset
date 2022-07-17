#pragma once
#include <vector>
#include <functional>  

namespace bunshi {
    
    template<typename... T>
    class Signal {

        public:
            void connect(std::function<void(T...)> func) {

                FunctionSignature signature;
                signature.type_id = typeid(func).hash_code();
                signature.instance_pointer = nullptr;
                signature.function = func;

                add(signature);
            }

            template<typename Function, typename InstanceType>
            void connect(Function func, InstanceType& instance) {

                FunctionSignature signature;
                signature.type_id = typeid(func).hash_code();
                signature.instance_pointer = (void*)&instance;
                signature.function = std::bind(func, &instance, (typeid(T), std::placeholders::_1)...);

                add(signature);
            }

            void disconnect(std::function<void(T...)> func) {
                FunctionSignature signature;
                signature.type_id = typeid(func).hash_code();
                signature.instance_pointer = nullptr;

                remove(signature);
            }

            template<typename Function, typename InstanceType>
            void disconnect(Function func, InstanceType& instance) {

                FunctionSignature signature;
                signature.type_id = typeid(func).hash_code();
                signature.instance_pointer = (void*)&instance;

                remove(signature);
            }

            void emit(T... values) {
                for(auto signature : functions) {
                    signature.function(values...);
                }
            }

        private:

            struct FunctionSignature {
                size_t type_id;
                void* instance_pointer;
                std::function<void(T...)> function;
            };

            void add(FunctionSignature& signature) {

                bool exist = false;
                for(auto other_signature : functions) {
                    if(other_signature.type_id == signature.type_id && other_signature.instance_pointer == signature.instance_pointer) {
                        exist = true;
                        break;
                    }
                }
                if(!exist) {
                    functions.push_back(signature);
                }
            }

            void remove(FunctionSignature& signature) {
                for(size_t i = 0; i < functions.size(); i++) {
                    if(functions[i].type_id == signature.type_id && functions[i].instance_pointer == signature.instance_pointer) {
                        
                        //function found. remove it and break
                        functions.erase(functions.begin() + i);
                        break;
                    }
                }
            }

            std::vector<FunctionSignature> functions;
    };
}
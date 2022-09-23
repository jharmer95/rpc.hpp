///@file rpc_adapters/rpc_rapidjson.hpp
///@author Jackson Harmer (jharmer95@gmail.com)
///@brief Implementation of adapting rapidjson (https://github.com/Tencent/rapidjson)
///
///@copyright
///BSD 3-Clause License
///
///Copyright (c) 2020-2022, Jackson Harmer
///All rights reserved.
///
///Redistribution and use in source and binary forms, with or without
///modification, are permitted provided that the following conditions are met:
///
///1. Redistributions of source code must retain the above copyright notice, this
///   list of conditions and the following disclaimer.
///
///2. Redistributions in binary form must reproduce the above copyright notice,
///   this list of conditions and the following disclaimer in the documentation
///   and/or other materials provided with the distribution.
///
///3. Neither the name of the copyright holder nor the names of its
///   contributors may be used to endorse or promote products derived from
///   this software without specific prior written permission.
///
///THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
///AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
///IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
///DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
///FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
///DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
///SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
///CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
///OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
///

#ifndef RPC_ADAPTERS_RAPIDJSON_HPP
#define RPC_ADAPTERS_RAPIDJSON_HPP

// #include "../rpc.hpp"

// #include <rapidjson/document.h>
// #include <rapidjson/stringbuffer.h>
// #include <rapidjson/writer.h>

// namespace rpc_hpp
// {
// namespace adapters
// {
//     class rapidjson_adapter;

//     template<>
//     struct serial_traits<rapidjson_adapter>
//     {
//         using serial_t = rapidjson::Document;
//         using bytes_t = std::string;
//     };

//     class rapidjson_adapter : public detail::serial_adapter_base<rapidjson_adapter>
//     {
//     public:
//         [[nodiscard]] static std::string to_bytes(rapidjson::Document&& serial_obj)
//         {
//             rapidjson::StringBuffer buffer{};
//             rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
//             std::move(serial_obj).Accept(writer);
//             return buffer.GetString();
//         }

//         [[nodiscard]] static std::optional<rapidjson::Document> from_bytes(std::string&& bytes)
//         {
//             rapidjson::Document d{};
//             d.SetObject();
//             d.Parse(std::move(bytes).c_str());

//             if (d.HasParseError())
//             {
//                 return std::nullopt;
//             }

//             if (const auto ex_it = d.FindMember("except_type"); ex_it != d.MemberEnd())
//             {
//                 if (const auto& ex_val = ex_it->value;
//                     !ex_val.IsInt() || (ex_val.GetInt() != 0 && !d.HasMember("err_mesg")))
//                 {
//                     return std::nullopt;
//                 }

//                 // Objects with exceptions can be otherwise empty
//                 return std::make_optional(std::move(d));
//             }

//             if (const auto fname_it = d.FindMember("func_name"); fname_it == d.MemberEnd()
//                 || !fname_it->value.IsString() || fname_it->value.GetStringLength() == 0)
//             {
//                 return std::nullopt;
//             }

//             if (const auto args_it = d.FindMember("args");
//                 args_it == d.MemberEnd() || !args_it->value.IsArray())
//             {
//                 return std::nullopt;
//             }

//             return std::make_optional(std::move(d));
//         }

//         static rapidjson::Document empty_object()
//         {
//             rapidjson::Document d{};
//             d.SetObject();
//             return d;
//         }

//         template<typename R, typename... Args>
//         [[nodiscard]] static rapidjson::Document serialize_pack(
//             const detail::packed_func<R, Args...>& pack)
//         {
//             rapidjson::Document d{};
//             auto& alloc = d.GetAllocator();
//             d.SetObject();
//             d.AddMember("func_name",
//                 rapidjson::Value{}.SetString(pack.get_func_name().c_str(), alloc), alloc);

//             if constexpr (!std::is_void_v<R>)
//             {
//                 rapidjson::Value result{};

//                 if (pack)
//                 {
//                     if constexpr (std::is_arithmetic_v<R>)
//                     {
//                         result.Set<R>(pack.get_result());
//                     }
//                     else if constexpr (std::is_same_v<R, std::string>)
//                     {
//                         result.SetString(pack.get_result().c_str(), alloc);
//                     }
//                     else if constexpr (rpc_hpp::detail::is_container_v<R>)
//                     {
//                         const auto& container = pack.get_result();
//                         result.SetArray();
//                         result.Reserve(static_cast<rapidjson::SizeType>(container.size()), alloc);

//                         for (const auto& val : container)
//                         {
//                             result.PushBack(val, alloc);
//                         }
//                     }
//                     else if constexpr (rpc_hpp::detail::is_serializable_v<rapidjson_adapter, R>)
//                     {
//                         const rapidjson::Document tmp =
//                             R::template serialize<rapidjson_adapter>(pack.get_result());

//                         result.CopyFrom(tmp, alloc);
//                     }
//                     else
//                     {
//                         result = serialize<R>(pack.get_result(), alloc);
//                     }
//                 }

//                 d.AddMember("result", result, alloc);
//             }

//             rapidjson::Value args{};
//             args.SetArray();
//             args.Reserve(static_cast<rapidjson::SizeType>(sizeof...(Args)), alloc);
//             detail::for_each_tuple(pack.get_args(),
//                 [&args, &alloc](auto&& elem)
//                 { push_args(std::forward<decltype(elem)>(elem), args, alloc); });

//             d.AddMember("args", std::move(args), alloc);
//             return d;
//         }

//         template<typename R, typename... Args>
//         [[nodiscard]] static detail::packed_func<R, Args...> deserialize_pack(
//             const rapidjson::Document& serial_obj)
//         {
//             const auto& args_val = serial_obj["args"];
//             [[maybe_unused]] unsigned arg_counter = 0;
//             typename detail::packed_func<R, Args...>::args_t args{ parse_args<Args>(
//                 args_val, arg_counter)... };

//             if constexpr (std::is_void_v<R>)
//             {
//                 detail::packed_func<void, Args...> pack(
//                     serial_obj["func_name"].GetString(), std::move(args));

//                 if (serial_obj.HasMember("except_type"))
//                 {
//                     pack.set_exception(serial_obj["err_mesg"].GetString(),
//                         static_cast<exception_type>(serial_obj["except_type"].GetInt()));
//                 }

//                 return pack;
//             }
//             else
//             {
//                 if (serial_obj.HasMember("result") && !serial_obj["result"].IsNull())
//                 {
//                     const rapidjson::Value& result = serial_obj["result"];
//                     return detail::packed_func<R, Args...>(
//                         serial_obj["func_name"].GetString(), parse_arg<R>(result), std::move(args));
//                 }

//                 detail::packed_func<R, Args...> pack(
//                     serial_obj["func_name"].GetString(), std::nullopt, std::move(args));

//                 if (serial_obj.HasMember("except_type"))
//                 {
//                     pack.set_exception(serial_obj["err_mesg"].GetString(),
//                         static_cast<exception_type>(serial_obj["except_type"].GetInt()));
//                 }

//                 return pack;
//             }
//         }

//         [[nodiscard]] static std::string get_func_name(const rapidjson::Document& serial_obj)
//         {
//             return serial_obj["func_name"].GetString();
//         }

//         [[nodiscard]] static rpc_exception extract_exception(const rapidjson::Document& serial_obj)
//         {
//             return rpc_exception{ serial_obj["err_mesg"].GetString(),
//                 static_cast<exception_type>(serial_obj["except_type"].GetInt()) };
//         }

//         static void set_exception(rapidjson::Document& serial_obj, const rpc_exception& ex)
//         {
//             auto& alloc = serial_obj.GetAllocator();

//             if (const auto ex_it = serial_obj.FindMember("except_type");
//                 ex_it != serial_obj.MemberEnd())
//             {
//                 ex_it->value.SetInt(static_cast<int>(ex.get_type()));
//                 serial_obj["err_mesg"].SetString(ex.what(), alloc);
//             }
//             else
//             {
//                 serial_obj.AddMember("except_type", static_cast<int>(ex.get_type()), alloc);
//                 serial_obj.AddMember(
//                     "err_mesg", rapidjson::Value{}.SetString(ex.what(), alloc), alloc);
//             }
//         }

//         template<typename T>
//         static rapidjson::Value serialize(
//             const T& val, rapidjson::MemoryPoolAllocator<>& alloc) = delete;

//         template<typename T>
//         static T deserialize(const rapidjson::Value& serial_obj) = delete;

//     private:
//         // nodiscard because this function is pointless without checking the bool
//         template<typename T>
//         [[nodiscard]] static constexpr bool validate_arg(const rapidjson::Value& arg) noexcept
//         {
//             if constexpr (std::is_same_v<T, bool>)
//             {
//                 return arg.IsBool();
//             }
//             else if constexpr (std::is_integral_v<T>)
//             {
//                 if constexpr (std::is_signed_v<T>)
//                 {
//                     if constexpr (sizeof(T) < 8)
//                     {
//                         return arg.IsInt();
//                     }
//                     else
//                     {
//                         return arg.IsInt64();
//                     }
//                 }
//                 else
//                 {
//                     if constexpr (sizeof(T) < 8)
//                     {
//                         return arg.IsUint();
//                     }
//                     else
//                     {
//                         return arg.IsUint64();
//                     }
//                 }
//             }
//             else if constexpr (std::is_same_v<T, float>)
//             {
//                 return arg.IsFloat();
//             }
//             else if constexpr (std::is_same_v<T, double>)
//             {
//                 return arg.IsDouble();
//             }
//             else if constexpr (std::is_same_v<T, std::string>)
//             {
//                 return arg.IsString();
//             }
//             else if constexpr (rpc_hpp::detail::is_container_v<T>)
//             {
//                 return arg.IsArray();
//             }
//             else
//             {
//                 return !arg.IsNull();
//             }
//         }

//         // nodiscard because expect_type is consumed by the function
//         [[nodiscard]] static std::string mismatch_message(
//             std::string&& expect_type, const rapidjson::Value& obj)
//         {
//             const auto get_type_str = [&obj]() noexcept
//             {
//                 if (obj.IsNull())
//                 {
//                     return "null";
//                 }

//                 if (obj.IsInt64())
//                 {
//                     return "int64";
//                 }

//                 if (obj.IsInt())
//                 {
//                     return "int";
//                 }

//                 if (obj.IsUint64())
//                 {
//                     return "uint64";
//                 }

//                 if (obj.IsUint())
//                 {
//                     return "uint";
//                 }

//                 if (obj.IsDouble())
//                 {
//                     return "double";
//                 }

//                 if (obj.IsFloat())
//                 {
//                     return "float";
//                 }

//                 if (obj.IsBool())
//                 {
//                     return "bool";
//                 }

//                 if (obj.IsString())
//                 {
//                     return "string";
//                 }

//                 if (obj.IsArray())
//                 {
//                     return "array";
//                 }

//                 if (obj.IsObject())
//                 {
//                     return "object";
//                 }

//                 return "unknown";
//             };

//             return { "rapidjson expected type: " + std::move(expect_type)
//                 + ", got type: " + get_type_str() };
//         }

//         template<typename T>
//         static void push_arg(
//             T&& arg, rapidjson::Value& obj, rapidjson::MemoryPoolAllocator<>& alloc)
//         {
//             using no_ref_t = detail::remove_cvref_t<T>;

//             if constexpr (std::is_same_v<no_ref_t, std::string>)
//             {
//                 obj.SetString(arg.c_str(), alloc);
//             }
//             else if constexpr (std::is_arithmetic_v<no_ref_t>)
//             {
//                 // Rapidjson does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
//                 if constexpr (
//                     std::is_same_v<no_ref_t,
//                         char> || std::is_same_v<no_ref_t, int8_t> || std::is_same_v<no_ref_t, int16_t>)
//                 {
//                     obj.SetInt(arg);
//                 }
//                 else if constexpr (std::is_same_v<no_ref_t,
//                                        uint8_t> || std::is_same_v<no_ref_t, uint16_t>)
//                 {
//                     obj.SetUint(arg);
//                 }
//                 else
//                 {
//                     obj.Set<no_ref_t>(arg);
//                 }
//             }
//             else if constexpr (rpc_hpp::detail::is_container_v<no_ref_t>)
//             {
//                 obj.SetArray();
//                 obj.Reserve(static_cast<rapidjson::SizeType>(arg.size()), alloc);

//                 for (auto&& val : arg)
//                 {
//                     push_args(std::forward<decltype(val)>(val), obj, alloc);
//                 }
//             }
//             else if constexpr (rpc_hpp::detail::is_serializable_v<rapidjson_adapter, no_ref_t>)
//             {
//                 const rapidjson::Document serialized =
//                     no_ref_t::template serialize<rapidjson_adapter>(std::forward<T>(arg));

//                 obj.CopyFrom(serialized, alloc);
//             }
//             else
//             {
//                 obj = serialize<no_ref_t>(std::forward<T>(arg), alloc);
//             }
//         }

//         template<typename T>
//         static void push_args(
//             T&& arg, rapidjson::Value& obj_arr, rapidjson::MemoryPoolAllocator<>& alloc)
//         {
//             rapidjson::Value tmp{};
//             push_arg(std::forward<T>(arg), tmp, alloc);
//             obj_arr.PushBack(std::move(tmp), alloc);
//         }

//         // nodiscard because parsing can be expensive, and it makes no sense to not use the parsed result
//         template<typename T>
//         [[nodiscard]] static detail::remove_cvref_t<T> parse_arg(
//             const rapidjson::Value& arg)
//         {
//             using no_ref_t = detail::remove_cvref_t<T>;

//             if (!validate_arg<no_ref_t>(arg))
//             {
//                 throw function_mismatch(mismatch_message(typeid(no_ref_t).name(), arg));
//             }

//             if constexpr (std::is_same_v<no_ref_t, std::string>)
//             {
//                 return arg.GetString();
//             }
//             else if constexpr (std::is_arithmetic_v<no_ref_t>)
//             {
//                 // Rapidjson does not have generic support for 8/16 bit numbers, so upgrade to 32-bit
//                 if constexpr (
//                     std::is_same_v<no_ref_t,
//                         char> || std::is_same_v<no_ref_t, int8_t> || std::is_same_v<no_ref_t, int16_t>)
//                 {
//                     return static_cast<no_ref_t>(arg.GetInt());
//                 }
//                 else if constexpr (std::is_same_v<no_ref_t,
//                                        uint8_t> || std::is_same_v<no_ref_t, uint16_t>)
//                 {
//                     return static_cast<no_ref_t>(arg.GetUint());
//                 }
//                 else
//                 {
//                     return arg.Get<no_ref_t>();
//                 }
//             }
//             else if constexpr (rpc_hpp::detail::is_container_v<no_ref_t>)
//             {
//                 using subvalue_t = typename no_ref_t::value_type;

//                 no_ref_t container{};
//                 container.reserve(arg.Size());
//                 unsigned arg_counter = 0;

//                 for (const auto& val : arg.GetArray())
//                 {
//                     container.push_back(parse_args<subvalue_t>(val, arg_counter));
//                 }

//                 return container;
//             }
//             else if constexpr (rpc_hpp::detail::is_serializable_v<rapidjson_adapter, no_ref_t>)
//             {
//                 rapidjson::Document d{};
//                 d.CopyFrom(arg, d.GetAllocator());
//                 return no_ref_t::template deserialize<rapidjson_adapter>(d);
//             }
//             else
//             {
//                 return deserialize<no_ref_t>(arg);
//             }
//         }

//         // nodiscard because parsing can be expensive, and it makes no sense to not use the parsed result
//         template<typename T>
//         [[nodiscard]] static detail::remove_cvref_t<T> parse_args(
//             const rapidjson::Value& arg_arr, unsigned& index)
//         {
//             if (!arg_arr.IsArray())
//             {
//                 return parse_arg<T>(arg_arr);
//             }

//             const auto& arr = arg_arr.GetArray();

//             if (index >= arr.Size())
//             {
//                 throw function_mismatch("Argument count mismatch");
//             }

//             return parse_arg<T>(arr[index++]);
//         }
//     };
// } // namespace adapters
// } // namespace rpc_hpp

#endif

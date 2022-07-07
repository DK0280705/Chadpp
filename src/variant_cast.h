#pragma once
#include <variant>

template<class... Args>
struct Variant_cast_proxy
{
  std::variant<Args...> v;
  
  template<class... ToArgs>
  constexpr operator std::variant<ToArgs...>() const
  {
    return std::visit([](auto&& arg) -> std::variant<ToArgs...> {
        if constexpr (std::is_convertible_v<decltype(arg), std::variant<ToArgs...>>)
            return arg;
        else throw std::exception();
    }, v);
  }
};

template<class... Args>
constexpr Variant_cast_proxy<Args...> variant_cast(const std::variant<Args...>& v)
{
  return Variant_cast_proxy<Args...>{v};
}

template<class... Args>
constexpr Variant_cast_proxy<Args...> variant_cast(std::variant<Args...>&& v)
{
  return Variant_cast_proxy<Args...>{std::forward(v)};
}

#if !defined( NANO_RPC_STRING_CONVERSION_HPP__ )
#define NANO_RPC_STRING_CONVERSION_HPP__

#include <string>

namespace NanoRpc {

void Utf8ToWideString( const std::string &source, std::wstring *destination );
void WideToUtf8String( const std::wstring &source, std::string *destination );


} // namespace

#endif // NANO_RPC_STRING_CONVERSION_HPP__

// This file contains routines used to translate between UTF-8 and UTF-16LE.
//
// Notes on UTF-8:
//
//   Byte-0    Byte-1    Byte-2    Byte-3    Value
//  0xxxxxxx                                 00000000 00000000 0xxxxxxx
//  110yyyyy  10xxxxxx                       00000000 00000yyy yyxxxxxx
//  1110zzzz  10yyyyyy  10xxxxxx             00000000 zzzzyyyy yyxxxxxx
//  11110uuu  10uuzzzz  10yyyyyy  10xxxxxx   000uuuuu zzzzyyyy yyxxxxxx
//
//
// Notes on UTF-16:  (with wwww+1==uuuuu)
//
//      Word-0               Word-1          Value
//  110110ww wwzzzzyy   110111yy yyxxxxxx    000uuuuu zzzzyyyy yyxxxxxx
//  zzzzyyyy yyxxxxxx                        00000000 zzzzyyyy yyxxxxxx
//
//
// BOM or Byte Order Mark:
//     0xff 0xfe   little-endian utf-16 follows
//     0xfe 0xff   big-endian utf-16 follows
//

#include "string_conversion.hpp"

#include <cassert>
#include <string>

namespace NanoRpc {
namespace {

/*
	This lookup table is used to help decode the first byte of
	a multi-byte UTF8 character.
*/
static const unsigned char Utf8TranslationTable[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x00, 0x01, 0x02, 0x03, 0x00, 0x01, 0x00, 0x00,
};

} // namespace


// The function translates UTF-8 to UTF-16 string.
// source - Source string
// destination - Destrination string. The string is cleared before conversion.
//
// The function assumes that size of integer type is at least 32-bit.
// 
// Notes on invalid UTF-8:
//
//  *  This routine never allows a 7-bit character (0x00 through 0x7f) to
//     be encoded as a multi-byte character.  Any multi-byte character that
//     attempts to encode a value between 0x00 and 0x7f is rendered as 0xfffd.
//
//  *  This routine never allows a UTF16 surrogate value to be encoded.
//     If a multi-byte character attempts to encode a value between
//     0xd800 and 0xe000 then it is rendered as 0xfffd.
//
//  *  Bytes in the range of 0x80 through 0xbf which occur as the first
//     byte of a character are interpreted as single-byte characters
//     and rendered as themselves even though they are technically
//     invalid characters.
//
//  *  This routine accepts an infinite number of different UTF8 encodings
//     for unicode values 0x80 and greater.  It do not change over-length
//     encodings to 0xfffd as some systems recommend.
//
void Utf8ToWideString( const std::string &source, std::wstring *destination )
{
	assert( sizeof( unsigned int ) >= 4 );
	assert( destination != NULL );

	destination->clear();
	destination->reserve( source.length() );

	for( std::string::const_iterator iter = source.begin(); iter != source.end(); ) {
		// Read UTF-8 sequence
		unsigned int c = *iter ++;
		if( c >= 0xc0 ) {
			c = Utf8TranslationTable[c - 0xc0];
			while( iter != source.end() && (*iter & 0xc0) == 0x80 ) {
				c = (c << 6) + (0x3f & *(iter ++));
			}
			if( c < 0x80 || (c & 0xFFFFF800) == 0xD800 || (c & 0xFFFFFFFE) == 0xFFFE ) {
				c = 0xFFFD;   // REPLACEMENT CHARACTER
			}
		}

		// Write UTF-16 sequence
		if( c <= 0xFFFF ) {
			*destination += (wchar_t)(c & 0xFFFF);
		}
		else {
			*destination += (wchar_t)(0xD800 + (((c - 0x10000) >> 10) & 0x03FF));
			*destination += (wchar_t)(0xDC00 + (c & 0x03FF));
		}
	}
}


// The function translates UTF-16 to UTF-8 string.
// source - Source string
// destination - Destrination string. The string is cleared before conversion.
//
// The function assumes that size of integer type is at least 32-bit.
// 
// Notes on invalid sequences:
//
//  *  If UTF16 sequence is not complete, the incomplete character
//     is replaced with as 0xfffd.
//
void WideToUtf8String( const std::wstring &source, std::string *destination )
{
	assert( sizeof( unsigned int ) >= 4 );
	assert( destination != NULL );

	destination->clear();
	destination->reserve( source.length() * 2 );  // pessimistic reservation

	for( std::wstring::const_iterator iter = source.begin(); iter != source.end(); ) {

		// Read UTF-16 sequence.
		unsigned int c = *iter ++;
		if( c >= 0xD800 && c < 0xE000 ) {
			if( iter != source.end() ) {
				unsigned int c2 = *iter ++;
				c = (c2 & 0x03FF) + ((c & 0x003F) << 10) + (((c & 0x03C0) + 0x0040) << 10);
			}
			else {
				c = 0xFFFD;   // REPLACEMENT CHARACTER
			}
		}

		// Write UTF-8 sequence.
		if( c < 0x00080 ) {
			*destination += (char)(c & 0xFF);
		}
		else if( c < 0x00800 ) {
			*destination += 0xC0 + (char)((c >> 6) & 0x1F);
			*destination += 0x80 + (char)(c & 0x3F);
		}
		else if( c < 0x10000 ) {
			*destination += 0xE0 + (char)((c >> 12) & 0x0F);
			*destination += 0x80 + (char)((c >> 6) & 0x3F);
			*destination += 0x80 + (char)(c & 0x3F);
		}
		else {
			*destination += 0xF0 + (char)((c >> 18) & 0x07);
			*destination += 0x80 + (char)((c >> 12) & 0x3F);
			*destination += 0x80 + (char)((c >> 6) & 0x3F);
			*destination += 0x80 + (char)(c & 0x3F);
		}
	}
}


} // namespace


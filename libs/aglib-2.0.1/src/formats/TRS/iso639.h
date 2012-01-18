/**
*  ISO639  language codes
*/

namespace ISO639 {
    
const char* get3LetterCode(const char* two_letter_code);
const char* get2LetterCode(const char* three_letter_code);
const char* getLanguageName(const char* three_letter_code, const char* locale="fre");

} /* namespace ISO639 */

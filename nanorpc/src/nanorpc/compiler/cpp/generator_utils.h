#if !defined(NANORPC_GENERATOR_UTILS_H__)
#define NANORPC_GENERATOR_UTILS_H__

#include <string>
#include <vector>

bool StripSuffix(std::string *filename, const std::string &suffix);
std::string StripProto(std::string filename);
std::string FilenameIdentifier(const std::string &filename);
std::vector<std::string> tokenize(const std::string &input,
                                  const std::string &delimiters);

#endif  // NANORPC_GENERATOR_UTILS_H__

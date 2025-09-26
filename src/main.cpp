#include <filesystem>
#include <iostream>
#include <string>
#include "transpiler/transpiler.h"

void printUsage(const char *programName) {
  std::cout << "Roblox C++ to Luau Transpiler\n";
  std::cout << "Usage: " << programName << " [options] <input_file.cpp>\n\n";
  std::cout << "Options:\n";
  std::cout << "  -o <output_file>  Specify output file (default: tests/output/output.luau)\n";
  std::cout << "  -b, --bootstrap   Generate bootstrapper file (default: tests/output/bootstrap.luau)\n";
  std::cout << "  -v, --verbose     Enable verbose output\n";
  std::cout << "  -h, --help        Show this help message\n";
  std::cout << "\nExample:\n";
  std::cout << "  " << programName << " -o tests/output/game.luau -b tests/input/main.cpp\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printUsage(argv[0]);
    return 1;
  }
  std::string inputFile;
  std::string outputFile;
  bool verbose = false;
  bool generateBootstrapper = false;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-h" || arg == "--help") {
      printUsage(argv[0]);
      return 0;
    } else if (arg == "-v" || arg == "--verbose") {
      verbose = true;
    } else if (arg == "-b" || arg == "--bootstrap") {
      generateBootstrapper = true;
    } else if (arg == "-o" && i + 1 < argc) {
      outputFile = argv[++i];
    } else if (arg[0] != '-') {
      if (inputFile.empty()) {
        inputFile = arg;
      } else {
        std::cerr << "Error: Multiple input files specified. Only one is supported.\n";
        return 1;
      }
    } else {
      std::cerr << "Error: Unknown option: " << arg << "\n";
      printUsage(argv[0]);
      return 1;
    }
  }

  if (inputFile.empty()) {
    std::cerr << "Error: No input file specified.\n";
    printUsage(argv[0]);
    return 1;
  }

  // Check if input file exists
  if (!std::filesystem::exists(inputFile)) {
    std::cerr << "Error: Input file does not exist: " << inputFile << "\n";
    return 1;
  }

  // Create transpiler and process file
  roblox_transpiler::Transpiler transpiler;
  transpiler.setVerbose(verbose);

  if (verbose) {
    std::cout << "Transpiling: " << inputFile << "\n";
  }

  bool success = transpiler.transpileFile(inputFile, outputFile);
  if (!success) {
    std::cerr << "Transpilation failed: " << transpiler.getLastError() << "\n";
    return 1;
  }
  // Generate bootstrapper if requested
  if (generateBootstrapper) {
    std::string bootstrapFile = "tests/output/bootstrap.luau";

    if (bool bootstrapSuccess = transpiler.generateBootstrapper(inputFile, bootstrapFile)) {
      if (verbose) {
        std::cout << "Bootstrapper generated: " << bootstrapFile << "\n";
      }
    } else {
      std::cerr << "Warning: Failed to generate bootstrapper\n";
    }
  }

  // Also output to console if verbose
  if (verbose) {
    std::cout << "\n--- Generated Luau Code ---\n";
    std::string luauCode = transpiler.transpileToString(inputFile);
    std::cout << luauCode << "\n";
    std::cout << "--- End Generated Code ---\n";
  }

  return 0;
}

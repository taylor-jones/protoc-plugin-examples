#include "generator.h"

int main(int argc, char* argv[]) {
  Generator generator;
  PluginMain(argc, argv, &generator);
  return 0;
}
find . -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
  -not -path "./*build*/*" \
  -exec clang-format -style=Microsoft -i {} +

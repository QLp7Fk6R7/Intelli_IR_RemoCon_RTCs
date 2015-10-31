FILE(REMOVE_RECURSE
  "doc"
  "CMakeFiles/build"
)

# Per-language clean rules from dependency scanning.
FOREACH(lang)
  INCLUDE(CMakeFiles/build.dir/cmake_clean_${lang}.cmake OPTIONAL)
ENDFOREACH(lang)

# Tests CMakeLists.txt

# Simple test framework (no external dependencies)
add_executable(test_runner
    test_main.cpp
    test_types.cpp
    test_atomspace.cpp
    test_attention.cpp
    test_pattern.cpp
    test_pln.cpp
    test_endocrine.cpp
    test_integration.cpp
    test_marduk.cpp
    test_touchpad.cpp
    test_atomese.cpp
    test_nervous.cpp
    test_neuroendocrine.cpp
    test_temporal.cpp
    test_social.cpp
    test_civic_angel.cpp
    test_afi.cpp
    test_entelechy_integration.cpp
    test_cloninger.cpp
    test_interoceptive.cpp
    test_developmental.cpp
    test_narrative.cpp
)

target_link_libraries(test_runner opencog_core)

# Add test
add_test(NAME opencog_tests COMMAND test_runner)

add_executable(classic classic.cc)
cctz_target_set_cxx_standard(classic)
target_link_libraries(classic)

add_executable(hello hello.cc)
cctz_target_set_cxx_standard(hello)
target_link_libraries(hello cctz::cctz)

add_executable(epoch_shift epoch_shift.cc)
cctz_target_set_cxx_standard(epoch_shift)
target_link_libraries(epoch_shift cctz::cctz)

add_executable(example1 example1.cc)
cctz_target_set_cxx_standard(example1)
target_link_libraries(example1 cctz::cctz)

add_executable(example2 example2.cc)
cctz_target_set_cxx_standard(example2)
target_link_libraries(example2 cctz::cctz)

add_executable(example3 example3.cc)
cctz_target_set_cxx_standard(example3)
target_link_libraries(example3 cctz::cctz)

add_executable(example4 example4.cc)
cctz_target_set_cxx_standard(example4)
target_link_libraries(example4 cctz::cctz)

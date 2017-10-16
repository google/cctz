add_executable(classic classic.cc)
target_link_libraries(classic)

add_executable(hello hello.cc)
target_link_libraries(hello cctz::cctz)

add_executable(epoch_shift epoch_shift.cc)
target_link_libraries(epoch_shift cctz::cctz)

add_executable(example1 example1.cc)
target_link_libraries(example1 cctz::cctz)

add_executable(example2 example2.cc)
target_link_libraries(example2 cctz::cctz)

add_executable(example3 example3.cc)
target_link_libraries(example3 cctz::cctz)

add_executable(example4 example4.cc)
target_link_libraries(example4 cctz::cctz)

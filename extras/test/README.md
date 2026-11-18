# Host tests

Tests that compile the library on a PC, using a minimal `Arduino.h` shim.
Run them once for each hash size from the library folder:

```sh
for HASH in uint8_t uint16_t uint32_t; do
  c++ -std=c++11 -Wall -I extras/test -I src -DSCPI_HASH_TYPE=$HASH \
      -DTEST_SETUP_ERRORS_API extras/test/test_parser.cpp -o test_parser &&
  ./test_parser || break
done
```

Without `-DTEST_SETUP_ERRORS_API` the tests that use `GetSetupErrors()` and
`HasSetupError()` are skipped, so the other tests can also be built with
older versions of the library.

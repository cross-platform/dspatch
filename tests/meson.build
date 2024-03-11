# Configure tests

dspatch_tests_src = [
    'main.cpp'
]

dspatch_tests_inc = include_directories('.')

dspatch_tests = executable(
    'Tests',
    format_first,
    dspatch_tests_src,
    include_directories: dspatch_tests_inc,
    dependencies: dspatch_dep
)

# Add code coverage

if opencppcoverage.found()
    test('Tests', opencppcoverage,
        args: [
            opencppcoverage_args,
            '--export_type', 'binary:Tests.cov',
            dspatch_tests
        ], timeout: 120
    )
else
    test('Tests', dspatch_tests, timeout: 120)
endif

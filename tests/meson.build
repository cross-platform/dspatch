# Configure catch

catch = subproject('catch')
catch_dep = catch.get_variable('catch_dep')

# Configure tests

dspatch_tests_src = [
    'main.cpp'
]

dspatch_tests_inc = include_directories('.')

dspatch_tests = executable(
    'DSPatchTests',
    format_first,
    dspatch_tests_src,
    include_directories: dspatch_tests_inc,
    dependencies: [dspatch_dep, catch_dep]
)

# Add code coverage

if opencppcoverage.found()
    test('DSPatchTests', opencppcoverage,
        args: [
            opencppcoverage_args,
            '--export_type', 'binary:DSPatchTests.cov',
            dspatch_tests
        ]
    )
else
    test('DSPatchTests', dspatch_tests)
endif

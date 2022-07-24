project('dspatch', 'cpp',
    default_options: [
        'cpp_std=c++11',
        'warning_level=3',
        'werror=true'
    ]
)

# Add code coverage

opencppcoverage = find_program('opencppcoverage', required: false)

opencppcoverage_args = [
    '--sources', meson.project_name(),
    '--excluded_sources', 'subprojects',
    '--excluded_sources', 'tests'
]

if opencppcoverage.found()
    run_target('codecoverage',
        command: [
            opencppcoverage,
            '--input_coverage', 'DSPatchTests.cov',
            '--export_type', 'html:coverage'
        ]
    )
endif

# Add formatting

format_first = []
if find_program('clang-format', required: false).found()
    if host_machine.system() == 'windows'
        format_command = meson.current_source_dir() + '/scripts/clang-format.bat'
    else
        format_command = ['bash', meson.current_source_dir() + '/scripts/clang-format.sh']
    endif

    format_first = custom_target(
        output: 'formatting',
        command: format_command,
        build_by_default: true
    )
endif

# Add static code analysis

if find_program('cppcheck', required: false).found()
    if host_machine.system() == 'windows'
        extra_cppcheck_args = ['-D_WIN32']
    else
        extra_cppcheck_args = []
    endif

    custom_target(
        output: 'static code analysis',
        command: [
            'cppcheck',

            '--error-exitcode=1',
            '--enable=all',
            '--inconclusive',
            '--inline-suppr',
            '--force',
            '--quiet',
            '--suppress=missingIncludeSystem',
            '--suppress=unmatchedSuppression',
            '--suppress=containerOutOfBounds',
            '--suppress=cstyleCast',
            '--suppress=useStlAlgorithm',

            '-i', meson.current_source_dir() + '/builddir',
            '-i', meson.current_source_dir() + '/subprojects',

            '-I', meson.current_source_dir() + '/include',
            '-I', meson.current_source_dir() + '/src',
            '-I', meson.current_source_dir() + '/tests',

            extra_cppcheck_args,

            meson.current_source_dir()
        ],
        build_by_default: true
    )
endif

# Configure DSPatch

dspatch_src = [
    'src/internal/AutoTickThread.cpp',
    'src/internal/CircuitThread.cpp',
    'src/internal/ComponentThread.cpp',
    'src/Circuit.cpp',
    'src/Component.cpp',
    'src/Plugin.cpp',
    'src/Signal.cpp',
    'src/SignalBus.cpp'
]

dspatch_inc = include_directories(
    'include',
    'src'
)

dspatch_lib = static_library(
    'DSPatch',
    format_first,
    dspatch_src,
    include_directories: dspatch_inc
)

dspatch_dep = declare_dependency(
  link_with: dspatch_lib,
  include_directories: dspatch_inc,
  dependencies: dependency('threads')
)

# Add subdirectories

subdir('tests')
subdir('tutorial')
